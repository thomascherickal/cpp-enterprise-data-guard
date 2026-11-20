#include "admin_gui/admin_controller.hpp"
#include "test_framework.hpp"

#include <cstdio>
#include <string>

using namespace dataguard::admin;

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

