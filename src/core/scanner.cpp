#include "core/scanner.hpp"

#include "core/json.hpp"
#include "core/redactor.hpp"
#include "core/rule_registry.hpp"
#include "core/text_index.hpp"
#include "core/validators.hpp"

#include <algorithm>
#include <regex>
#include <utility>

namespace dataguard {

namespace {

bool isAllowlisted(const std::vector<std::string>& allowlist, const std::string& match) {
    for (const auto& entry : allowlist) {
        if (!entry.empty() && match.find(entry) != std::string::npos) {
            return true;
        }
    }
    return false;
}

void runRule(const std::string& input, const TextIndex& index, const Policy& policy,
             const std::string& id, const std::string& category, const std::string& severity,
             const std::string& description, const std::string& pattern, bool caseInsensitive,
             bool (*validator)(std::string_view), std::vector<Finding>& out) {
    if (severityRank(severity) < severityRank(policy.minSeverity)) {
        return;
    }
    auto flags = std::regex::ECMAScript;
    if (caseInsensitive) flags |= std::regex::icase;
    std::regex re(pattern, flags);

    auto begin = std::sregex_iterator(input.begin(), input.end(), re);
    auto end = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
        const auto& m = *it;
        std::string matched = m.str(0);
        if (matched.empty()) continue;
        if (isAllowlisted(policy.allowlist, matched)) continue;
        if (validator != nullptr && !validator(matched)) continue;

        std::string entropyTarget = (m.size() > 1 && m[1].matched) ? m.str(1) : matched;

        Finding f;
        f.ruleId = id;
        f.category = category;
        f.severity = severity;
        f.description = description;
        f.maskedMatch = validators::maskSecret(matched);
        f.start = static_cast<std::size_t>(m.position(0));
        f.end = f.start + static_cast<std::size_t>(m.length(0));
        index.locate(f.start, f.line, f.column);
        f.entropy = validators::shannonEntropy(entropyTarget);
        out.push_back(std::move(f));
    }
}

void scanEntropyBearers(const std::string& input, const TextIndex& index, const Policy& policy,
                        std::vector<Finding>& out) {
    if (!policy.enableEntropy || severityRank("HIGH") < severityRank(policy.minSeverity)) {
        return;
    }
    static const std::regex kBearerRe(
        R"(Authorization\s*:\s*Bearer\s+([A-Za-z0-9._~+/-]{20,}=*))",
        std::regex::ECMAScript | std::regex::icase);
    auto begin = std::sregex_iterator(input.begin(), input.end(), kBearerRe);
    auto end = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
        const auto& m = *it;
        std::string token = m.str(1);
        double ent = validators::shannonEntropy(token);
        if (ent >= policy.entropyThreshold && !isAllowlisted(policy.allowlist, token)) {
            Finding f;
            f.ruleId = "SEC-HIGH-ENTROPY-BEARER";
            f.category = "SECRET";
            f.severity = "HIGH";
            f.description = "High-Entropy Bearer Token";
            f.maskedMatch = validators::maskSecret(m.str(0));
            f.start = static_cast<std::size_t>(m.position(0));
            f.end = f.start + static_cast<std::size_t>(m.length(0));
            index.locate(f.start, f.line, f.column);
            f.entropy = ent;
            out.push_back(std::move(f));
        }
    }
}

std::vector<Finding> deduplicateFindings(std::vector<Finding>& raw) {
    std::sort(raw.begin(), raw.end(), [](const Finding& a, const Finding& b) {
        if (a.start != b.start) return a.start < b.start;
        int ra = severityRank(a.severity);
        int rb = severityRank(b.severity);
        if (ra != rb) return ra > rb;
        return (a.end - a.start) > (b.end - b.start);
    });

    std::vector<Finding> unique;
    std::size_t lastEnd = 0;
    for (auto& f : raw) {
        if (f.start < lastEnd) continue;
        lastEnd = f.end;
        unique.push_back(std::move(f));
    }
    return unique;
}

int computeRiskScore(const std::vector<Finding>& findings, bool& hasCritical) {
    int score = 0;
    hasCritical = false;
    for (const auto& f : findings) {
        int r = severityRank(f.severity);
        if (r == 4) {
            score += 40;
            hasCritical = true;
        } else if (r == 3) {
            score += 25;
        } else if (r == 2) {
            score += 15;
        } else {
            score += 5;
        }
    }
    return std::min(100, score);
}

}  // namespace

std::string ScanResult::toJson(int indent) const {
    auto root = json::Value::makeObject();
    root.set("blocked", json::Value(blocked));
    root.set("riskScore", json::Value(riskScore));
    root.set("findingCount", json::Value(static_cast<int>(findings.size())));
    root.set("redactedText", json::Value(redactedText));

    auto arr = json::Value::makeArray();
    for (const auto& f : findings) {
        auto item = json::Value::makeObject();
        item.set("ruleId", json::Value(f.ruleId));
        item.set("category", json::Value(f.category));
        item.set("severity", json::Value(f.severity));
        item.set("description", json::Value(f.description));
        item.set("maskedMatch", json::Value(f.maskedMatch));
        item.set("start", json::Value(static_cast<int>(f.start)));
        item.set("end", json::Value(static_cast<int>(f.end)));
        item.set("line", json::Value(static_cast<int>(f.line)));
        item.set("column", json::Value(static_cast<int>(f.column)));
        item.set("entropy", json::Value(f.entropy));
        arr.push(std::move(item));
    }
    root.set("findings", std::move(arr));
    return root.dump(indent);
}

Scanner::Scanner(Policy policy) : policy_(std::move(policy)) {}

void Scanner::setPolicy(Policy policy) { policy_ = std::move(policy); }

std::string Scanner::version() { return "1.0.0"; }

ScanResult Scanner::scan(const std::string& input) const {
    ScanResult result;
    if (input.empty()) {
        return result;
    }

    TextIndex index(input);
    std::vector<Finding> raw;

    if (policy_.enableSecrets) {
        for (const auto& rule : RuleRegistry::secretRules()) {
            runRule(input, index, policy_, rule.id, rule.category, rule.severity,
                    rule.description, rule.pattern, rule.caseInsensitive, rule.validator, raw);
        }
        scanEntropyBearers(input, index, policy_, raw);
    }

    if (policy_.enablePii) {
        for (const auto& rule : RuleRegistry::piiRules()) {
            runRule(input, index, policy_, rule.id, rule.category, rule.severity,
                    rule.description, rule.pattern, rule.caseInsensitive, rule.validator, raw);
        }
        if (policy_.enableIpAddresses) {
            const auto& ip = RuleRegistry::ipRule();
            runRule(input, index, policy_, ip.id, ip.category, ip.severity, ip.description,
                    ip.pattern, ip.caseInsensitive, ip.validator, raw);
        }
    }

    for (const auto& cr : policy_.customRules) {
        if (!cr.enabled || cr.id.empty() || cr.pattern.empty()) continue;
        try {
            runRule(input, index, policy_, cr.id, cr.category, cr.severity, cr.description,
                    cr.pattern, false, nullptr, raw);
        } catch (const std::regex_error&) {
            // Skip invalid custom regex gracefully at scan time
        }
    }

    result.findings = deduplicateFindings(raw);
    bool hasCritical = false;
    result.riskScore = computeRiskScore(result.findings, hasCritical);
    result.blocked = (policy_.blockOnCritical && hasCritical) ||
                     (policy_.maxFindingsBeforeBlock > 0 &&
                      static_cast<int>(result.findings.size()) >= policy_.maxFindingsBeforeBlock);
    result.redactedText = Redactor::apply(input, result.findings, policy_.redactionMode);
    return result;
}

std::string Scanner::redact(const std::string& input) const {
    return scan(input).redactedText;
}

}  // namespace dataguard
