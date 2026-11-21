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

  CHECK(ctrl.runTestScan("key=AKIA1234567890ABCDEF").blocked);

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
  CHECK(
      !loader.loadPolicyFromFile("/tmp/nonexistent_dg_policy_12345.json", err));

  {
    std::ofstream ofs(tmpPath);
    ofs << "{bad";
  }
  CHECK(!loader.loadPolicyFromFile(tmpPath, err));

  {
    std::ofstream ofs(tmpPath);
    ofs << R"({"minSeverity":"INVALID"})";
  }
  CHECK(!loader.loadPolicyFromFile(tmpPath, err));

  dataguard::Policy bad = ctrl.policy();
  bad.organization = "";
  ctrl.setPolicy(bad);
  CHECK(!ctrl.savePolicyToFile(tmpPath, err));

  ctrl.resetToDefaults();
  CHECK(!ctrl.savePolicyToFile("/nonexistent_dir_xyz/policy.json", err));
  CHECK(!ctrl.savePolicyToFile("/dev/full", err));
  std::remove(tmpPath.c_str());
}
