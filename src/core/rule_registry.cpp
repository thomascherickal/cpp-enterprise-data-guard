#include "core/rule_registry.hpp"

#include "core/validators.hpp"

namespace dataguard {

const std::vector<RuleDefinition>& RuleRegistry::secretRules() {
    static const std::vector<RuleDefinition> kRules = {
        {"SEC-AWS-KEY", "SECRET", "CRITICAL", "AWS Access Key ID",
         R"(\bAKIA[0-9A-Z]{16}\b)", false, nullptr},
        {"SEC-GITHUB-TOKEN", "SECRET", "CRITICAL", "GitHub Personal Access Token",
         R"(\b(?:ghp|gho|ghu|ghs|ghr)_[A-Za-z0-9]{36}\b|\bgithub_pat_[A-Za-z0-9_]{22,82}\b)",
         false, nullptr},
        {"SEC-ANTHROPIC-KEY", "SECRET", "CRITICAL", "Anthropic API Key",
         R"(\bsk-ant-[A-Za-z0-9_-]{20,}\b)", false, nullptr},
        {"SEC-OPENAI-KEY", "SECRET", "CRITICAL", "OpenAI API Key",
         R"(\bsk-(?:proj-|svcacct-)?[A-Za-z0-9_-]{20,}\b)", false, nullptr},
        {"SEC-GOOGLE-API-KEY", "SECRET", "HIGH", "Google API Key",
         R"(\bAIza[0-9A-Za-z_-]{35}\b)", false, nullptr},
        {"SEC-STRIPE-KEY", "SECRET", "CRITICAL", "Stripe Live Secret Key",
         R"(\b(?:sk|rk)_live_[0-9A-Za-z]{24,}\b)", false, nullptr},
        {"SEC-SLACK-TOKEN", "SECRET", "HIGH", "Slack API Token",
         R"(\bxox[baprs]-[0-9A-Za-z-]{10,48}\b)", false, nullptr},
        {"SEC-PRIVATE-KEY", "SECRET", "CRITICAL", "PEM Private Key Header",
         R"(-----BEGIN (?:RSA |EC |OPENSSH |PGP |DSA )?PRIVATE KEY-----)", false, nullptr},
        {"SEC-JWT", "SECRET", "MEDIUM", "JSON Web Token (JWT)",
         R"(\beyJ[A-Za-z0-9_-]{10,}\.[A-Za-z0-9_-]{10,}\.[A-Za-z0-9_-]{10,}\b)",
         false, nullptr},
        {"SEC-DB-URI", "SECRET", "CRITICAL", "Database Connection URI with Credentials",
         R"((?:postgres|postgresql|mysql|mongodb(?:\+srv)?|redis):\/\/[^:\s/]+:[^@\s/]+@[^\s]+)",
         true, nullptr},
        {"SEC-GENERIC-ASSIGN", "SECRET", "HIGH", "Hardcoded Secret or Password Assignment",
         R"((?:password|passwd|pwd|secret|api[_-]?key|access[_-]?token|auth[_-]?token)\s*[:=]\s*["']([^"'\n]{8,})["'])",
         true, nullptr},
    };
    return kRules;
}

const std::vector<RuleDefinition>& RuleRegistry::piiRules() {
    static const std::vector<RuleDefinition> kRules = {
        {"PII-CREDIT-CARD", "PII", "CRITICAL", "Payment Card Number (Luhn Validated)",
         R"(\b(?:\d[ -]*?){13,19}\b)", false, validators::isValidLuhn},
        {"PII-SSN", "PII", "CRITICAL", "US Social Security Number",
         R"(\b\d{3}-\d{2}-\d{4}\b)", false, validators::isValidSsn},
        {"PII-IBAN", "PII", "HIGH", "International Bank Account Number (IBAN)",
         R"(\b[A-Z]{2}\d{2}[A-Z0-9 ]{11,30}\b)", false, validators::isValidIban},
        {"PII-EMAIL", "PII", "MEDIUM", "Email Address",
         R"(\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}\b)", false, nullptr},
        {"PII-PHONE", "PII", "LOW", "Phone Number",
         R"((?:\+\d{1,3}[-.\s]?)?\(?\d{3}\)?[-.\s]\d{3}[-.\s]\d{4}\b)", false, nullptr},
    };
    return kRules;
}

const RuleDefinition& RuleRegistry::ipRule() {
    static const RuleDefinition kRule = {
        "PII-IPV4", "PII", "LOW", "IPv4 Address",
        R"(\b(?:(?:25[0-5]|2[0-4]\d|[01]?\d\d?)\.){3}(?:25[0-5]|2[0-4]\d|[01]?\d\d?)\b)",
        false, nullptr};
    return kRule;
}

}  // namespace dataguard

