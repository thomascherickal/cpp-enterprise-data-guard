#pragma once

#include <string>
#include <vector>

namespace dataguard {

struct CustomRule {
    std::string id;
    std::string pattern;
    std::string category = "SECRET";
    std::string severity = "HIGH";
    std::string description;
    bool enabled = true;
};

struct Policy {
    std::string policyVersion = "1.0";
    std::string organization = "Enterprise Default";
    bool enableSecrets = true;
    bool enablePii = true;
    bool enableEntropy = true;
    bool enableIpAddresses = false;
    std::string minSeverity = "LOW";
    double entropyThreshold = 3.8;
    std::string redactionMode = "PLACEHOLDER";  // PLACEHOLDER, MASK, REMOVE
    bool blockOnCritical = true;
    int maxFindingsBeforeBlock = 1;
    std::vector<std::string> allowlist;
    std::vector<CustomRule> customRules;

    static Policy defaultEnterprise();
    static Policy fromJson(const std::string& jsonStr, std::string& error);
    std::string toJson(int indent = 2) const;
    bool validate(std::vector<std::string>& errors) const;
};

int severityRank(const std::string& severity);

}  // namespace dataguard

