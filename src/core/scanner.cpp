#include "core/scanner.hpp"

#include "core/json.hpp"
#include "core/validators.hpp"

#include <algorithm>
#include <regex>
#include <utility>

namespace dataguard {

namespace {

struct BuiltinRule {
    const char* id;
    const char* category;
    const char* severity;
    const char* description;
    const char* pattern;
    bool caseInsensitive;
    bool (*validator)(std::string_view);
};

const BuiltinRule kSecretRules[] = {
    {"SEC-AWS-KEY", "SECRET", "CRITICAL", "AWS Access Key ID",
     R"(\bAKIA[0-9A-Z]{16}\b)", false, nullptr},
    {"SEC-GITHUB-TOKEN", "SECRET", "CRITICAL", "GitHub Personal Access Token",
     R"(\b(?:ghp|gho|ghu|ghs|ghr)_[A-Za-z0-9]{36}\b|\bgithub_pat_[A-Za-z0-9_]{22,82}\b)",
     false, nullptr},
    {"SEC-ANTHROPIC-KEY", "SECRET", "CRITICAL", "Anthropic API Key",
     R"(\bsk-ant-[A-Za-z0-9_-]{20,}\b)", false, nullptr},
    {"SEC-OPENAI-KEY", "SECRET", "CRITICAL", "OpenAI API Key",
     R"(\bsk-(?:proj-|svcacct-)?[A-Za-z0-9_-]{20,}\b)", false, nullptr},
    {"SEC-GOOGLE-API-KEY", "SECRET", "HIGH", "Google API Key",
     R"(\bAIza[0-9A-Za-z_-]{35}\b)", false, nullptr},
    {"SEC-STRIPE-KEY", "SECRET", "CRITICAL", "Stripe Live Secret Key",
     R"(\b(?:sk|rk)_live_[0-9A-Za-z]{24,}\b)", false, nullptr},
    {"SEC-SLACK-TOKEN", "SECRET", "HIGH", "Slack API Token",
     R"(\bxox[baprs]-[0-9A-Za-z-]{10,48}\b)", false, nullptr},
    {"SEC-PRIVATE-KEY", "SECRET", "CRITICAL", "PEM Private Key Header",
     R"(-----BEGIN (?:RSA |EC |OPENSSH |PGP |DSA )?PRIVATE KEY-----)", false, nullptr},
    {"SEC-JWT", "SECRET", "MEDIUM", "JSON Web Token (JWT)",
     R"(\beyJ[A-Za-z0-9_-]{10,}\.[A-Za-z0-9_-]{10,}\.[A-Za-z0-9_-]{10,}\b)",
     false, nullptr},
    {"SEC-DB-URI", "SECRET", "CRITICAL", "Database Connection URI with Credentials",
     R"((?:postgres|postgresql|mysql|mongodb(?:\+srv)?|redis):\/\/[^:\s/]+:[^@\s/]+@[^\s]+)",
     true, nullptr},
    {"SEC-GENERIC-ASSIGN", "SECRET", "HIGH", "Hardcoded Secret or Password Assignment",
     R"((?:password|passwd|pwd|secret|api[_-]?key|access[_-]?token|auth[_-]?token)\s*[:=]\s*["']([^"'\n]{8,})["'])",
     true, nullptr},
};

const BuiltinRule kPiiRules[] = {
    {"PII-CREDIT-CARD", "PII", "CRITICAL", "Payment Card Number (Luhn Validated)",
     R"(\b(?:\d[ -]*?){13,19}\b)", false, validators::isValidLuhn},
    {"PII-SSN", "PII", "CRITICAL", "US Social Security Number",
     R"(\b\d{3}-\d{2}-\d{4}\b)", false, validators::isValidSsn},
    {"PII-IBAN", "PII", "HIGH", "International Bank Account Number (IBAN)",
     R"(\b[A-Z]{2}\d{2}[A-Z0-9 ]{11,30}\b)", false, validators::isValidIban},
    {"PII-EMAIL", "PII", "MEDIUM", "Email Address",
     R"(\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}\b)", false, nullptr},
    {"PII-PHONE", "PII", "LOW", "Phone Number",
     R"((?:\+\d{1,3}[-.\s]?)?\(?\d{3}\)?[-.\s]\d{3}[-.\s]\d{4}\b)", false, nullptr},
};

const BuiltinRule kIpRule = {
    "PII-IPV4", "PII", "LOW", "IPv4 Address",
    R"(\b(?:(?:25[0-5]|2[0-4]\d|[01]?\d\d?)\.){3}(?:25[0-5]|2[0-4]\d|[01]?\d\d?)\b)",
    false, nullptr};

std::vector<std::size_t> computeLineStarts(const std::string& text) {
    std::vector<std::size_t> starts = {0};
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\n') {
            starts.push_back(i + 1);
        }
    }
    return starts;
}

void locateOffset(const std::vector<std::size_t>& lineStarts, std::size_t offset,
                  std::size_t& outLine, std::size_t& outCol) {
    auto it = std::upper_bound(lineStarts.begin(), lineStarts.end(), offset);
    std::size_t lineIdx = static_cast<std::size_t>(it - lineStarts.begin()) - 1;
    outLine = lineIdx + 1;
    outCol = (offset - lineStarts[lineIdx]) + 1;
}

bool isAllowlisted(const std::vector<std::string>& allowlist, const std::string& match) {
    for (const auto& entry : allowlist) {
        if (!entry.empty() && match.find(entry) != std::string::npos) {
            return true;
        }
    }
    return false;
}

void runRule(const std::string& input, const std::vector<std::size_t>& lineStarts,
             const Policy& policy, const std::string& id, const std::string& category,
             const std::string& severity, const std::string& description,
             const std::string& pattern, bool caseInsensitive,
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
        double entropy = validators::shannonEntropy(entropyTarget);

        Finding f;
        f.ruleId = id;
        f.category = category;
        f.severity = severity;
        f.description = description;
        f.maskedMatch = validators::maskSecret(matched);
        f.start = static_cast<std::size_t>(m.position(0));
        f.end = f.start + static_cast<std::size_t>(m.length(0));
        locateOffset(lineStarts, f.start, f.line, f.column);
        f.entropy = entropy;
        out.push_back(std::move(f));
    }
}

std::string buildRedactedText(const std::string& input, const std::vector<Finding>& findings,
                              const std::string& mode) {
    std::string out;
    out.reserve(input.size());
    std::size_t cursor = 0;
    for (const auto& f : findings) {
        if (f.start > cursor) {
            out.append(input, cursor, f.start - cursor);
        }
        if (mode == "REMOVE") {
            // Omit matched secret/PII
        } else if (mode == "MASK") {
            out.append(f.maskedMatch);
        } else {
            out.append("[REDACTED:");
            out.append(f.ruleId);
            out.push_back(']');
        }
        cursor = f.end;
    }
    if (cursor < input.size()) {
        out.append(input, cursor, input.size() - cursor);
    }
    return out;
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

    auto lineStarts = computeLineStarts(input);
    std::vector<Finding> raw;

    if (policy_.enableSecrets) {
        for (const auto& rule : kSecretRules) {
            runRule(input, lineStarts, policy_, rule.id, rule.category, rule.severity,
                    rule.description, rule.pattern, rule.caseInsensitive, rule.validator, raw);
        }
        if (policy_.enableEntropy) {
            static const std::regex kBearerRe(
                R"(Authorization\s*:\s*Bearer\s+([A-Za-z0-9._~+/-]{20,}=*))",
                std::regex::ECMAScript | std::regex::icase);
            auto begin = std::sregex_iterator(input.begin(), input.end(), kBearerRe);
            auto end = std::sregex_iterator();
            for (auto it = begin; it != end; ++it) {
                const auto& m = *it;
                std::string token = m.str(1);
                double ent = validators::shannonEntropy(token);
                if (ent >= policy_.entropyThreshold &&
                    !isAllowlisted(policy_.allowlist, token) &&
                    severityRank("HIGH") >= severityRank(policy_.minSeverity)) {
                    Finding f;
                    f.ruleId = "SEC-HIGH-ENTROPY-BEARER";
                    f.category = "SECRET";
                    f.severity = "HIGH";
                    f.description = "High-Entropy Bearer Token";
                    f.maskedMatch = validators::maskSecret(m.str(0));
                    f.start = static_cast<std::size_t>(m.position(0));
                    f.end = f.start + static_cast<std::size_t>(m.length(0));
                    locateOffset(lineStarts, f.start, f.line, f.column);
                    f.entropy = ent;
                    raw.push_back(std::move(f));
                }
            }
        }
    }

    if (policy_.enablePii) {
        for (const auto& rule : kPiiRules) {
            runRule(input, lineStarts, policy_, rule.id, rule.category, rule.severity,
                    rule.description, rule.pattern, rule.caseInsensitive, rule.validator, raw);
        }
        if (policy_.enableIpAddresses) {
            runRule(input, lineStarts, policy_, kIpRule.id, kIpRule.category, kIpRule.severity,
                    kIpRule.description, kIpRule.pattern, kIpRule.caseInsensitive, kIpRule.validator,
                    raw);
        }
    }

    for (const auto& cr : policy_.customRules) {
        if (!cr.enabled || cr.id.empty() || cr.pattern.empty()) continue;
        try {
            runRule(input, lineStarts, policy_, cr.id, cr.category, cr.severity, cr.description,
                    cr.pattern, false, nullptr, raw);
        } catch (const std::regex_error&) {
            // Skip invalid custom regex gracefully at scan time
        }
    }

    // Sort by start ascending, then severity descending, then longer span first
    std::sort(raw.begin(), raw.end(), [](const Finding& a, const Finding& b) {
        if (a.start != b.start) return a.start < b.start;
        int ra = severityRank(a.severity);
        int rb = severityRank(b.severity);
        if (ra != rb) return ra > rb;
        return (a.end - a.start) > (b.end - b.start);
    });

    // Deduplicate overlapping matches
    std::size_t lastEnd = 0;
    for (auto& f : raw) {
        if (f.start < lastEnd) continue;
        lastEnd = f.end;
        result.findings.push_back(std::move(f));
    }

    int score = 0;
    bool hasCritical = false;
    for (const auto& f : result.findings) {
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
    result.riskScore = std::min(100, score);
    result.blocked = (policy_.blockOnCritical && hasCritical) ||
                     (policy_.maxFindingsBeforeBlock > 0 &&
                      static_cast<int>(result.findings.size()) >= policy_.maxFindingsBeforeBlock);
    result.redactedText = buildRedactedText(input, result.findings, policy_.redactionMode);
    return result;
}

std::string Scanner::redact(const std::string& input) const {
    return scan(input).redactedText;
}

}  // namespace dataguard

