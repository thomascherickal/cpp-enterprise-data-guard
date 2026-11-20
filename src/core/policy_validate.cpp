#include "core/policy.hpp"

#include "core/json.hpp"

#include <regex>
#include <utility>

namespace dataguard {

std::string Policy::toJson(int indent) const {
    auto root = json::Value::makeObject();
    root.set("policyVersion", json::Value(policyVersion));
    root.set("organization", json::Value(organization));
    root.set("enableSecrets", json::Value(enableSecrets));
    root.set("enablePii", json::Value(enablePii));
    root.set("enableEntropy", json::Value(enableEntropy));
    root.set("enableIpAddresses", json::Value(enableIpAddresses));
    root.set("minSeverity", json::Value(minSeverity));
    root.set("entropyThreshold", json::Value(entropyThreshold));
    root.set("redactionMode", json::Value(redactionMode));
    root.set("blockOnCritical", json::Value(blockOnCritical));
    root.set("maxFindingsBeforeBlock", json::Value(maxFindingsBeforeBlock));

    auto allowArr = json::Value::makeArray();
    for (const auto& entry : allowlist) allowArr.push(json::Value(entry));
    root.set("allowlist", std::move(allowArr));

    auto rulesArr = json::Value::makeArray();
    for (const auto& r : customRules) {
        auto rObj = json::Value::makeObject();
        rObj.set("id", json::Value(r.id));
        rObj.set("pattern", json::Value(r.pattern));
        rObj.set("category", json::Value(r.category));
        rObj.set("severity", json::Value(r.severity));
        rObj.set("description", json::Value(r.description));
        rObj.set("enabled", json::Value(r.enabled));
        rulesArr.push(std::move(rObj));
    }
    root.set("customRules", std::move(rulesArr));
    return root.dump(indent);
}

bool Policy::validate(std::vector<std::string>& errors) const {
    errors.clear();
    if (organization.empty()) errors.push_back("organization must not be empty");
    if (severityRank(minSeverity) == 0) {
        errors.push_back("minSeverity must be one of LOW, MEDIUM, HIGH, CRITICAL");
    }
    if (entropyThreshold < 2.0 || entropyThreshold > 6.0) {
        errors.push_back("entropyThreshold must be between 2.0 and 6.0");
    }
    if (redactionMode != "PLACEHOLDER" && redactionMode != "MASK" && redactionMode != "REMOVE") {
        errors.push_back("redactionMode must be PLACEHOLDER, MASK, or REMOVE");
    }
    if (maxFindingsBeforeBlock < 0) errors.push_back("maxFindingsBeforeBlock must be >= 0");
    for (std::size_t i = 0; i < customRules.size(); ++i) {
        const auto& r = customRules[i];
        if (r.id.empty() || r.pattern.empty()) {
            errors.push_back("customRules[" + std::to_string(i) + "] requires id and pattern");
            continue;
        }
        if (severityRank(r.severity) == 0) {
            errors.push_back("customRules[" + std::to_string(i) + "] has invalid severity");
        }
        try {
            std::regex re(r.pattern);
        } catch (const std::regex_error&) {
            errors.push_back("customRules[" + std::to_string(i) + "] has invalid regex pattern");
        }
    }
    return errors.empty();
}

}  // namespace dataguard

