#pragma once

#include <string_view>
#include <vector>

namespace dataguard {

struct RuleDefinition {
    const char* id;
    const char* category;
    const char* severity;
    const char* description;
    const char* pattern;
    bool caseInsensitive;
    bool (*validator)(std::string_view);
};

class RuleRegistry {
public:
    static const std::vector<RuleDefinition>& secretRules();
    static const std::vector<RuleDefinition>& piiRules();
    static const RuleDefinition& ipRule();
};

}  // namespace dataguard

