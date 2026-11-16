#include "core/policy.hpp"

#include "core/json.hpp"

#include <regex>
#include <utility>

namespace dataguard {

int severityRank(const std::string& severity) {
    if (severity == "LOW") return 1;
    if (severity == "MEDIUM") return 2;
    if (severity == "HIGH") return 3;
    if (severity == "CRITICAL") return 4;
    return 0;
}

Policy Policy::defaultEnterprise() {
    Policy p;
    p.allowlist = {"AKIAIOSFODNN7EXAMPLE", "test@example.com", "user@example.org"};
    return p;
}

Policy Policy::fromJson(const std::string& jsonStr, std::string& error) {
    Policy p = defaultEnterprise();
    if (jsonStr.empty()) {
        error.clear();
        return p;
    }

    auto parsed = json::parse(jsonStr);
    if (!parsed.ok) {
        error = "Invalid JSON: " + parsed.error;
        return p;
    }
    if (!parsed.value.isObject()) {
        error = "Policy root must be a JSON object";
        return p;
    }

    const auto& obj = parsed.value;
    if (obj.contains("policyVersion") && obj.get("policyVersion").isString()) {
        p.policyVersion = obj.get("policyVersion").asString();
    }
    if (obj.contains("organization") && obj.get("organization").isString()) {
        p.organization = obj.get("organization").asString();
    }
    if (obj.contains("enableSecrets") && obj.get("enableSecrets").isBool()) {
        p.enableSecrets = obj.get("enableSecrets").asBool();
    }
    if (obj.contains("enablePii") && obj.get("enablePii").isBool()) {
        p.enablePii = obj.get("enablePii").asBool();
    }
    if (obj.contains("enableEntropy") && obj.get("enableEntropy").isBool()) {
        p.enableEntropy = obj.get("enableEntropy").asBool();
    }
    if (obj.contains("enableIpAddresses") && obj.get("enableIpAddresses").isBool()) {
        p.enableIpAddresses = obj.get("enableIpAddresses").asBool();
    }
    if (obj.contains("minSeverity") && obj.get("minSeverity").isString()) {
        p.minSeverity = obj.get("minSeverity").asString();
    }
    if (obj.contains("entropyThreshold") && obj.get("entropyThreshold").isNumber()) {
        p.entropyThreshold = obj.get("entropyThreshold").asNumber();
    }
    if (obj.contains("redactionMode") && obj.get("redactionMode").isString()) {
        p.redactionMode = obj.get("redactionMode").asString();
    }
    if (obj.contains("blockOnCritical") && obj.get("blockOnCritical").isBool()) {
        p.blockOnCritical = obj.get("blockOnCritical").asBool();
    }
    if (obj.contains("maxFindingsBeforeBlock") && obj.get("maxFindingsBeforeBlock").isNumber()) {
        p.maxFindingsBeforeBlock = obj.get("maxFindingsBeforeBlock").asInt();
    }
    if (obj.contains("allowlist") && obj.get("allowlist").isArray()) {
        p.allowlist.clear();
        for (const auto& item : obj.get("allowlist").asArray()) {
            if (item.isString()) {
                p.allowlist.push_back(item.asString());
            }
        }
    }
    if (obj.contains("customRules") && obj.get("customRules").isArray()) {
        p.customRules.clear();
        for (const auto& item : obj.get("customRules").asArray()) {
            if (!item.isObject()) continue;
            CustomRule rule;
            rule.id = item.get("id").asString();
            rule.pattern = item.get("pattern").asString();
            if (item.contains("category") && item.get("category").isString()) {
                rule.category = item.get("category").asString();
            }
            if (item.contains("severity") && item.get("severity").isString()) {
                rule.severity = item.get("severity").asString();
            }
            if (item.contains("description") && item.get("description").isString()) {
                rule.description = item.get("description").asString();
            }
            if (item.contains("enabled") && item.get("enabled").isBool()) {
                rule.enabled = item.get("enabled").asBool();
            }
            p.customRules.push_back(std::move(rule));
        }
    }

    error.clear();
    return p;
}

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
    for (const auto& entry : allowlist) {
        allowArr.push(json::Value(entry));
    }
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
    if (organization.empty()) {
        errors.push_back("organization must not be empty");
    }
    if (severityRank(minSeverity) == 0) {
        errors.push_back("minSeverity must be one of LOW, MEDIUM, HIGH, CRITICAL");
    }
    if (entropyThreshold < 2.0 || entropyThreshold > 6.0) {
        errors.push_back("entropyThreshold must be between 2.0 and 6.0");
    }
    if (redactionMode != "PLACEHOLDER" && redactionMode != "MASK" && redactionMode != "REMOVE") {
        errors.push_back("redactionMode must be PLACEHOLDER, MASK, or REMOVE");
    }
    if (maxFindingsBeforeBlock < 0) {
        errors.push_back("maxFindingsBeforeBlock must be >= 0");
    }
    for (std::size_t i = 0; i < customRules.size(); ++i) {
        const auto& r = customRules[i];
        if (r.id.empty() || r.pattern.empty()) {
            errors.push_back("customRules[" + std::to_string(i) + "] requires non-empty id and pattern");
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

