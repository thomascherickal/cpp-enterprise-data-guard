#include "core/policy.hpp"

#include "core/json.hpp"

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
    if (jsonStr.empty()) { error.clear(); return p; }

    auto parsed = json::parse(jsonStr);
    if (!parsed.ok) { error = "Invalid JSON: " + parsed.error; return p; }
    if (!parsed.value.isObject()) { error = "Policy root must be a JSON object"; return p; }

    const auto& obj = parsed.value;
    auto readStr = [&](const char* k, std::string& dst) {
        if (obj.contains(k) && obj.get(k).isString()) dst = obj.get(k).asString();
    };
    auto readBool = [&](const char* k, bool& dst) {
        if (obj.contains(k) && obj.get(k).isBool()) dst = obj.get(k).asBool();
    };
    readStr("policyVersion", p.policyVersion);
    readStr("organization", p.organization);
    readBool("enableSecrets", p.enableSecrets);
    readBool("enablePii", p.enablePii);
    readBool("enableEntropy", p.enableEntropy);
    readBool("enableIpAddresses", p.enableIpAddresses);
    readStr("minSeverity", p.minSeverity);
    if (obj.contains("entropyThreshold") && obj.get("entropyThreshold").isNumber()) {
        p.entropyThreshold = obj.get("entropyThreshold").asNumber();
    }
    readStr("redactionMode", p.redactionMode);
    readBool("blockOnCritical", p.blockOnCritical);
    if (obj.contains("maxFindingsBeforeBlock") && obj.get("maxFindingsBeforeBlock").isNumber()) {
        p.maxFindingsBeforeBlock = obj.get("maxFindingsBeforeBlock").asInt();
    }
    if (obj.contains("allowlist") && obj.get("allowlist").isArray()) {
        p.allowlist.clear();
        for (const auto& item : obj.get("allowlist").asArray()) {
            if (item.isString()) p.allowlist.push_back(item.asString());
        }
    }
    if (obj.contains("customRules") && obj.get("customRules").isArray()) {
        p.customRules.clear();
        for (const auto& item : obj.get("customRules").asArray()) {
            if (!item.isObject()) continue;
            CustomRule rule;
            rule.id = item.get("id").asString();
            rule.pattern = item.get("pattern").asString();
            if (item.get("category").isString()) rule.category = item.get("category").asString();
            if (item.get("severity").isString()) rule.severity = item.get("severity").asString();
            if (item.get("description").isString()) {
                rule.description = item.get("description").asString();
            }
            if (item.get("enabled").isBool()) rule.enabled = item.get("enabled").asBool();
            p.customRules.push_back(std::move(rule));
        }
    }
    error.clear();
    return p;
}

}  // namespace dataguard

