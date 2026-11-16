#include "core/scanner.hpp"
#include "test_framework.hpp"

using namespace dataguard;

TEST_CASE("scanner empty and safe text") {
    Scanner scanner;
    CHECK(!Scanner::version().empty());
    CHECK_EQ(scanner.policy().organization, "Enterprise Default");

    auto emptyRes = scanner.scan("");
    CHECK(!emptyRes.blocked);
    CHECK_EQ(emptyRes.riskScore, 0);
    CHECK(emptyRes.findings.empty());

    auto safeRes = scanner.scan("How do I write a binary search in C++?");
    CHECK(!safeRes.blocked);
    CHECK_EQ(safeRes.riskScore, 0);
    CHECK_EQ(safeRes.redactedText, "How do I write a binary search in C++?");
}

TEST_CASE("scanner detects secrets and redacts") {
    Scanner scanner;
    std::string input =
        "aws=AKIA1234567890ABCDEF\n"
        "gh=ghp_1234567890abcdef1234567890abcdef1234\n"
        "ant=sk-ant-api03-1234567890abcdefghij123456\n"
        "oai=sk-proj-1234567890abcdefghij123456\n"
        "goog=AIzaSyDaGmWKa4JsXZ-HjGw7ISLn_3namBGt123\n"
        "stripe=" + std::string("sk_") + "live_1234567890abcdefghijklmnop\n"
        "slack=xoxb-1234567890-1234567890-abcdefghij\n"
        "-----BEGIN RSA PRIVATE KEY-----\n"
        "jwt=eyJhbGciOiJIUzI1NiJ9.eyJzdWIiOiIxMjM0NTY3ODkwIn0.dozjgNryP4J3jVmNHl0w5N_XgL0n3I9PlFUP0THsR8U\n"
        "db=postgres://admin:SuperPass123@db.internal:5432/app\n"
        "password = \"MySuperSecret99!\"\n"
        "Authorization: Bearer xK9mP2vL8nQ4zR7wT1yU5cB3eD6fG0hJ4kM8pQ2sV6xZ9\n";

    auto res = scanner.scan(input);
    CHECK(res.blocked);
    CHECK_EQ(res.riskScore, 100);
    CHECK(res.findings.size() >= 10u);
    CHECK(res.redactedText.find("AKIA1234567890ABCDEF") == std::string::npos);
    CHECK(res.redactedText.find("[REDACTED:SEC-AWS-KEY]") != std::string::npos);
    CHECK(res.toJson(-1).find("\"blocked\":true") != std::string::npos);
}

TEST_CASE("scanner detects pii and ipv4") {
    Policy p = Policy::defaultEnterprise();
    p.enableSecrets = false;
    p.enableIpAddresses = true;
    Scanner scanner(p);

    std::string input =
        "Card: 4532-0151-1283-0366\n"
        "SSN: 123-45-6789\n"
        "IBAN: DE89370400440532013000\n"
        "Email: jane.doe@company.com\n"
        "Phone: (555) 234-5678\n"
        "IP: 192.168.1.42\n";

    auto res = scanner.scan(input);
    CHECK(res.blocked);
    CHECK(res.findings.size() >= 5u);
    CHECK(res.redactedText.find("123-45-6789") == std::string::npos);
}

TEST_CASE("scanner allowlist minSeverity customRules and redactionModes") {
    Policy p = Policy::defaultEnterprise();
    p.minSeverity = "HIGH";
    p.redactionMode = "MASK";
    p.customRules.push_back({"CUST-DISABLED", "FOO", "CUSTOM", "HIGH", "", false});
    p.customRules.push_back({"CUST-BAD", "[unclosed", "CUSTOM", "HIGH", "", true});
    p.customRules.push_back({"CUST-INT", "INTERNAL_[0-9]{4}", "CUSTOM", "HIGH", "Internal", true});
    Scanner scanner(p);

    // Allowlisted AWS key + low/medium email should be skipped, custom rule should match
    std::string input =
        "allow=AKIAIOSFODNN7EXAMPLE email=user@example.org code=INTERNAL_9012";
    auto res = scanner.scan(input);
    CHECK_EQ(res.findings.size(), 1u);
    CHECK_EQ(res.findings[0].ruleId, "CUST-INT");
    CHECK(res.redactedText.find("INTE****9012") != std::string::npos);

    // Test REMOVE redaction mode + non-blocking low severity
    p.minSeverity = "LOW";
    p.redactionMode = "REMOVE";
    p.blockOnCritical = false;
    p.maxFindingsBeforeBlock = 5;
    scanner.setPolicy(p);
    std::string removed = scanner.redact("Call 555-234-5678 now");
    CHECK_EQ(removed, "Call  now");
}

TEST_CASE("scanner deduplicates overlapping matches") {
    Policy p = Policy::defaultEnterprise();
    p.customRules.push_back({"CUST-LOW", "AKIA[0-9A-Z]{16}", "CUSTOM", "LOW", "Overlap", true});
    Scanner scanner(p);

    auto res = scanner.scan("key=AKIA1234567890ABCDEF");
    CHECK_EQ(res.findings.size(), 1u);
    CHECK_EQ(res.findings[0].ruleId, "SEC-AWS-KEY");
}
