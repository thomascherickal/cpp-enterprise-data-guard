#include "admin_gui/admin_controller.hpp"

#include <iostream>
#include <string>

namespace dataguard::admin {

int runHeadlessCli(int argc, char** argv) {
    AdminController controller;
    std::string policyPath;
    std::string exportPath;
    std::string scanText;
    bool hasScan = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--headless") {
            continue;
        } else if (arg == "--policy" && i + 1 < argc) {
            policyPath = argv[++i];
        } else if (arg == "--export-policy" && i + 1 < argc) {
            exportPath = argv[++i];
        } else if (arg == "--scan" && i + 1 < argc) {
            scanText = argv[++i];
            hasScan = true;
        } else if (arg == "--help") {
            std::cout << "Enterprise Data Guard Admin Control Centre\n"
                      << "Usage: dataguard-admin [--headless] [--policy <path>] "
                      << "[--export-policy <path>] [--scan <text>]\n";
            return 0;
        } else {
            std::cerr << "Unknown or incomplete argument: " << arg << '\n';
            return 2;
        }
    }

    if (!policyPath.empty()) {
        std::string err;
        if (!controller.loadPolicyFromFile(policyPath, err)) {
            std::cerr << "Error loading policy: " << err << '\n';
            return 1;
        }
    }
    if (!exportPath.empty()) {
        std::string err;
        if (!controller.savePolicyToFile(exportPath, err)) {
            std::cerr << "Error exporting policy: " << err << '\n';
            return 1;
        }
        std::cout << "Exported policy to " << exportPath << '\n';
    }
    if (hasScan) {
        ScanResult res = controller.runTestScan(scanText);
        std::cout << res.toJson(2) << '\n';
        return res.blocked ? 3 : 0;
    }
    return 0;
}

}  // namespace dataguard::admin

