#pragma once

#include "core/policy.hpp"
#include "core/scanner.hpp"

#include <string>
#include <vector>

namespace dataguard::admin {

class AdminController {
public:
    AdminController();

    const Policy& policy() const { return policy_; }
    void setPolicy(Policy policy);

    bool loadPolicyFromFile(const std::string& path, std::string& error);
    bool savePolicyToFile(const std::string& path, std::string& error) const;

    bool addAllowlistEntry(const std::string& entry);
    bool removeAllowlistEntry(const std::string& entry);

    bool addCustomRule(const CustomRule& rule, std::string& error);
    bool removeCustomRule(const std::string& ruleId);

    ScanResult runTestScan(const std::string& sampleText) const;
    std::vector<std::string> validateCurrentPolicy() const;
    void resetToDefaults();

private:
    Policy policy_;
};

int runHeadlessCli(int argc, char** argv);

}  // namespace dataguard::admin

