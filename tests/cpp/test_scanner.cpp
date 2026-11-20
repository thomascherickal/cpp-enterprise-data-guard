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
    CHECK(res.toJson(-1).find("\"blocked\":true") != std::string::npos);

    Policy placeholderPolicy = Policy::defaultEnterprise();
    placeholderPolicy.redactionMode = "PLACEHOLDER";
    Scanner placeholderScanner(placeholderPolicy);
    CHECK_EQ(placeholderScanner.redact("aws=AKIA1234567890ABCDEF"),
             "aws=[REDACTED:SEC-AWS-KEY]");
}
