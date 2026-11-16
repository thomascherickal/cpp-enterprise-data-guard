#pragma once

#include "core/policy.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace dataguard {

struct Finding {
    std::string ruleId;
    std::string category;  // SECRET, PII, CUSTOM
    std::string severity;  // LOW, MEDIUM, HIGH, CRITICAL
    std::string description;
    std::string maskedMatch;
    std::size_t start = 0;
    std::size_t end = 0;
    std::size_t line = 1;
    std::size_t column = 1;
    double entropy = 0.0;
};

struct ScanResult {
    bool blocked = false;
    int riskScore = 0;
    std::vector<Finding> findings;
    std::string redactedText;

    std::string toJson(int indent = 2) const;
};

class Scanner {
public:
    explicit Scanner(Policy policy = Policy::defaultEnterprise());

    void setPolicy(Policy policy);
    const Policy& policy() const { return policy_; }

    ScanResult scan(const std::string& input) const;
    std::string redact(const std::string& input) const;

    static std::string version();

private:
    Policy policy_;
};

}  // namespace dataguard

