#include "core/policy.hpp"
#include "test_framework.hpp"

using namespace dataguard;

TEST_CASE("policy severity ranks") {
    CHECK_EQ(severityRank("LOW"), 1);
    CHECK_EQ(severityRank("MEDIUM"), 2);
    CHECK_EQ(severityRank("HIGH"), 3);
    CHECK_EQ(severityRank("CRITICAL"), 4);
    CHECK_EQ(severityRank("UNKNOWN"), 0);
}

TEST_CASE("policy serialization roundtrip") {
    Policy p = Policy::defaultEnterprise();
    p.organization = "Acme Security";
    p.enableIpAddresses = true;
    p.minSeverity = "MEDIUM";
    p.entropyThreshold = 4.1;
    p.redactionMode = "MASK";
    p.maxFindingsBeforeBlock = 2;
    p.customRules.push_back({"CUST-1", "SECRET_[0-9]+", "CUSTOM", "HIGH", "Custom token", true});

    std::string jsonStr = p.toJson(2);
    std::string err;
    Policy loaded = Policy::fromJson(jsonStr, err);
    CHECK(err.empty());
    CHECK_EQ(loaded.organization, "Acme Security");
    CHECK_EQ(loaded.enableIpAddresses, true);
    CHECK_EQ(loaded.minSeverity, "MEDIUM");
    CHECK_NEAR(loaded.entropyThreshold, 4.1, 1e-9);
    CHECK_EQ(loaded.redactionMode, "MASK");
    CHECK_EQ(loaded.maxFindingsBeforeBlock, 2);
    CHECK_EQ(loaded.customRules.size(), 1u);
    CHECK_EQ(loaded.customRules[0].id, "CUST-1");

    std::vector<std::string> errors;
    CHECK(loaded.validate(errors));
    CHECK(errors.empty());
}

TEST_CASE("policy fromJson edge cases") {
    std::string err;
    Policy empty = Policy::fromJson("", err);
    CHECK(err.empty());
    CHECK_EQ(empty.organization, "Enterprise Default");

    Policy::fromJson("{bad json", err);
    CHECK(!err.empty());

    Policy::fromJson("[1, 2]", err);
    CHECK(!err.empty());

    // Non-object element in customRules is skipped gracefully
    Policy p = Policy::fromJson(R"({"customRules":[123]})", err);
    CHECK(err.empty());
    CHECK(p.customRules.empty());
}

TEST_CASE("policy validation errors") {
    Policy p = Policy::defaultEnterprise();
    p.organization = "";
    p.minSeverity = "INVALID";
    p.entropyThreshold = 1.0;
    p.redactionMode = "BOGUS";
    p.maxFindingsBeforeBlock = -1;
    p.customRules.push_back({"", "", "CUSTOM", "HIGH", "", true});
    p.customRules.push_back({"C2", "abc", "CUSTOM", "BADSEV", "", true});
    p.customRules.push_back({"C3", "[unclosed", "CUSTOM", "HIGH", "", true});

    std::vector<std::string> errors;
    CHECK(!p.validate(errors));
    CHECK_EQ(errors.size(), 8u);
}

