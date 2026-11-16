#include "admin_gui/admin_controller.hpp"
#include "test_framework.hpp"

#include <cstdio>
#include <fstream>
#include <string>

using namespace dataguard::admin;

TEST_CASE("admin controller allowlist and custom rules") {
    AdminController ctrl;
    CHECK(ctrl.validateCurrentPolicy().empty());

    CHECK(!ctrl.addAllowlistEntry(""));
    CHECK(ctrl.addAllowlistEntry("SAFE_TOKEN"));
    CHECK(!ctrl.addAllowlistEntry("SAFE_TOKEN"));
    CHECK(ctrl.removeAllowlistEntry("SAFE_TOKEN"));
    CHECK(!ctrl.removeAllowlistEntry("SAFE_TOKEN"));

    std::string err;
    dataguard::CustomRule r{"R1", "PROJ_[0-9]+", "CUSTOM", "HIGH", "Proj", true};
    CHECK(ctrl.addCustomRule(r, err));
    CHECK(!ctrl.addCustomRule(r, err));
    CHECK(!ctrl.addCustomRule({"R2", "[bad", "CUSTOM", "HIGH", "", true}, err));
    CHECK(ctrl.removeCustomRule("R1"));
    CHECK(!ctrl.removeCustomRule("R1"));

    auto scanRes = ctrl.runTestScan("key=AKIA1234567890ABCDEF");
    CHECK(scanRes.blocked);

    ctrl.resetToDefaults();
    CHECK_EQ(ctrl.policy().organization, "Enterprise Default");
}

TEST_CASE("admin controller file load and save") {
    AdminController ctrl;
    std::string err;
    std::string tmpPath = "/tmp/dg_test_policy.json";

    CHECK(ctrl.savePolicyToFile(tmpPath, err));
    CHECK(err.empty());

    AdminController loader;
    CHECK(loader.loadPolicyFromFile(tmpPath, err));
    CHECK(err.empty());

    CHECK(!loader.loadPolicyFromFile("/tmp/nonexistent_dg_policy_12345.json", err));

    // Invalid JSON file
    {
        std::ofstream ofs(tmpPath);
        ofs << "{bad";
    }
    CHECK(!loader.loadPolicyFromFile(tmpPath, err));

    // Valid JSON but invalid policy semantics
    {
        std::ofstream ofs(tmpPath);
        ofs << R"({"minSeverity":"INVALID"})";
    }
    CHECK(!loader.loadPolicyFromFile(tmpPath, err));

    // Saving invalid policy fails
    dataguard::Policy bad = ctrl.policy();
    bad.organization = "";
    ctrl.setPolicy(bad);
    CHECK(!ctrl.savePolicyToFile(tmpPath, err));

    // Unwritable path fails
    ctrl.resetToDefaults();
    CHECK(!ctrl.savePolicyToFile("/nonexistent_dir_xyz/policy.json", err));
    // Write failure on /dev/full (opens successfully, fails on flush/write)
    CHECK(!ctrl.savePolicyToFile("/dev/full", err));

    std::remove(tmpPath.c_str());
}

TEST_CASE("admin headless cli") {
    std::string tmpPolicy = "/tmp/dg_cli_policy.json";
    {
        const char* argv[] = {"dataguard-admin", "--help"};
        CHECK_EQ(runHeadlessCli(2, const_cast<char**>(argv)), 0);
    }
    {
        const char* argv[] = {"dataguard-admin", "--headless", "--export-policy", tmpPolicy.c_str()};
        CHECK_EQ(runHeadlessCli(4, const_cast<char**>(argv)), 0);
    }
    {
        const char* argv[] = {"dataguard-admin", "--policy", tmpPolicy.c_str(),
                              "--scan", "safe prompt"};
        CHECK_EQ(runHeadlessCli(5, const_cast<char**>(argv)), 0);
    }
    {
        const char* argv[] = {"dataguard-admin", "--scan", "AKIA1234567890ABCDEF"};
        CHECK_EQ(runHeadlessCli(3, const_cast<char**>(argv)), 3);
    }
    {
        const char* argv[] = {"dataguard-admin", "--unknown"};
        CHECK_EQ(runHeadlessCli(2, const_cast<char**>(argv)), 2);
    }
    {
        const char* argv[] = {"dataguard-admin", "--policy", "/tmp/no_such_file_dg.json"};
        CHECK_EQ(runHeadlessCli(3, const_cast<char**>(argv)), 1);
    }
    {
        const char* argv[] = {"dataguard-admin", "--export-policy", "/nonexistent_dir_xyz/p.json"};
        CHECK_EQ(runHeadlessCli(3, const_cast<char**>(argv)), 1);
    }
    std::remove(tmpPolicy.c_str());
}
