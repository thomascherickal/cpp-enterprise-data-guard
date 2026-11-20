#include "admin_gui/admin_controller.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <utility>

namespace dataguard::admin {

AdminController::AdminController() : policy_(Policy::defaultEnterprise()) {}

void AdminController::setPolicy(Policy policy) { policy_ = std::move(policy); }

bool AdminController::loadPolicyFromFile(const std::string& path, std::string& error) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) { error = "Unable to open policy file: " + path; return false; }
    std::ostringstream oss;
    oss << ifs.rdbuf();
    std::string parseErr;
    Policy loaded = Policy::fromJson(oss.str(), parseErr);
    if (!parseErr.empty()) { error = parseErr; return false; }
    std::vector<std::string> validationErrors;
    if (!loaded.validate(validationErrors)) { error = validationErrors.front(); return false; }
    policy_ = std::move(loaded);
    error.clear();
    return true;
}

bool AdminController::savePolicyToFile(const std::string& path, std::string& error) const {
    std::vector<std::string> validationErrors;
    if (!policy_.validate(validationErrors)) { error = validationErrors.front(); return false; }
    std::ofstream ofs(path);
    if (!ofs.is_open()) { error = "Unable to write policy file: " + path; return false; }
    ofs << policy_.toJson(2) << '\n';
    ofs.flush();
    if (!ofs.good()) { error = "Failed writing policy file: " + path; return false; }
    error.clear();
    return true;
}

bool AdminController::addAllowlistEntry(const std::string& entry) {
    if (entry.empty()) return false;
    if (std::find(policy_.allowlist.begin(), policy_.allowlist.end(), entry) !=
        policy_.allowlist.end()) {
        return false;
    }
    policy_.allowlist.push_back(entry);
    return true;
}

bool AdminController::removeAllowlistEntry(const std::string& entry) {
    auto it = std::find(policy_.allowlist.begin(), policy_.allowlist.end(), entry);
    if (it == policy_.allowlist.end()) return false;
    policy_.allowlist.erase(it);
    return true;
}

bool AdminController::addCustomRule(const CustomRule& rule, std::string& error) {
    for (const auto& existing : policy_.customRules) {
        if (existing.id == rule.id) { error = "Duplicate custom rule ID: " + rule.id; return false; }
    }
    Policy candidate = policy_;
    candidate.customRules.push_back(rule);
    std::vector<std::string> errors;
    if (!candidate.validate(errors)) { error = errors.front(); return false; }
    policy_ = std::move(candidate);
    error.clear();
    return true;
}

bool AdminController::removeCustomRule(const std::string& ruleId) {
    auto it = std::find_if(policy_.customRules.begin(), policy_.customRules.end(),
                           [&](const CustomRule& r) { return r.id == ruleId; });
    if (it == policy_.customRules.end()) return false;
    policy_.customRules.erase(it);
    return true;
}

ScanResult AdminController::runTestScan(const std::string& sampleText) const {
    Scanner scanner(policy_);
    return scanner.scan(sampleText);
}

std::vector<std::string> AdminController::validateCurrentPolicy() const {
    std::vector<std::string> errors;
    policy_.validate(errors);
    return errors;
}

void AdminController::resetToDefaults() { policy_ = Policy::defaultEnterprise(); }

}  // namespace dataguard::admin
