#include "core/scanner.hpp"
#include "test_framework.hpp"

using namespace dataguard;

TEST_CASE("scanner detects pii and ipv4") {
  Policy p = Policy::defaultEnterprise();
  p.enableSecrets = false;
  p.enableIpAddresses = true;
  Scanner scanner(p);

  std::string input = "Card: 4532-0151-1283-0366\n"
                      "SSN: 123-45-6789\n"
                      "InvalidSSN: 000-45-6789\n"
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
  p.customRules.push_back(
      {"CUST-DISABLED", "FOO", "CUSTOM", "HIGH", "", false});
  p.customRules.push_back(
      {"CUST-BAD", "[unclosed", "CUSTOM", "HIGH", "", true});
  p.customRules.push_back(
      {"CUST-INT", "INTERNAL_[0-9]{4}", "CUSTOM", "HIGH", "Internal", true});
  Scanner scanner(p);

  std::string input =
      "allow=AKIAIOSFODNN7EXAMPLE email=user@example.org code=INTERNAL_9012";
  auto res = scanner.scan(input);
  CHECK_EQ(res.findings.size(), 1u);
  CHECK_EQ(res.findings[0].ruleId, "CUST-INT");
  CHECK(res.redactedText.find("INTE****9012") != std::string::npos);

  p.minSeverity = "LOW";
  p.redactionMode = "REMOVE";
  p.blockOnCritical = false;
  p.maxFindingsBeforeBlock = 5;
  scanner.setPolicy(p);
  CHECK_EQ(scanner.redact("Call 555-234-5678 now"), "Call  now");
}

TEST_CASE(
    "scanner deduplicates overlapping matches and respects enableEntropy") {
  Policy p = Policy::defaultEnterprise();
  p.customRules.push_back(
      {"CUST-LOW", "AKIA[0-9A-Z]{16}", "CUSTOM", "LOW", "Overlap", true});
  Scanner scanner(p);

  auto res = scanner.scan("key=AKIA1234567890ABCDEF");
  CHECK_EQ(res.findings.size(), 1u);
  CHECK_EQ(res.findings[0].ruleId, "SEC-AWS-KEY");

  p.enableEntropy = false;
  scanner.setPolicy(p);
  auto noEnt = scanner.scan(
      "Authorization: Bearer xK9mP2vL8nQ4zR7wT1yU5cB3eD6fG0hJ4kM8pQ2sV6xZ9");
  CHECK_EQ(noEnt.findings.size(), 0u);
}
