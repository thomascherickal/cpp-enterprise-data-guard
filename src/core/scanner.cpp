#include "core/scanner.hpp"

#include "core/redactor.hpp"
#include "core/rule_registry.hpp"
#include "core/scanner_engine.hpp"
#include "core/text_index.hpp"

#include <regex>
#include <utility>

namespace dataguard {

Scanner::Scanner(Policy policy) : policy_(std::move(policy)) {}

void Scanner::setPolicy(Policy policy) { policy_ = std::move(policy); }

std::string Scanner::version() { return "1.0.0"; }

ScanResult Scanner::scan(const std::string& input) const {
    ScanResult result;
    if (input.empty()) return result;

    TextIndex index(input);
    std::vector<Finding> raw;

    if (policy_.enableSecrets) {
        for (const auto& r : RuleRegistry::secretRules()) {
            detail::runRule(input, index, policy_, r.id, r.category, r.severity, r.description,
                            r.pattern, r.caseInsensitive, r.validator, raw);
        }
        detail::scanEntropyBearers(input, index, policy_, raw);
    }

    if (policy_.enablePii) {
        for (const auto& r : RuleRegistry::piiRules()) {
            detail::runRule(input, index, policy_, r.id, r.category, r.severity, r.description,
                            r.pattern, r.caseInsensitive, r.validator, raw);
        }
        if (policy_.enableIpAddresses) {
            const auto& ip = RuleRegistry::ipRule();
            detail::runRule(input, index, policy_, ip.id, ip.category, ip.severity, ip.description,
                            ip.pattern, ip.caseInsensitive, ip.validator, raw);
        }
    }

    for (const auto& cr : policy_.customRules) {
        if (!cr.enabled || cr.id.empty() || cr.pattern.empty()) continue;
        try {
            detail::runRule(input, index, policy_, cr.id, cr.category, cr.severity, cr.description,
                            cr.pattern, false, nullptr, raw);
        } catch (const std::regex_error&) {
            // Skip invalid custom regex gracefully at scan time
        }
    }

    result.findings = detail::deduplicateFindings(raw);
    bool hasCritical = false;
    result.riskScore = detail::computeRiskScore(result.findings, hasCritical);
    result.blocked = (policy_.blockOnCritical && hasCritical) ||
                     (policy_.maxFindingsBeforeBlock > 0 &&
                      static_cast<int>(result.findings.size()) >= policy_.maxFindingsBeforeBlock);
    result.redactedText = Redactor::apply(input, result.findings, policy_.redactionMode);
    return result;
}

std::string Scanner::redact(const std::string& input) const { return scan(input).redactedText; }

}  // namespace dataguard
