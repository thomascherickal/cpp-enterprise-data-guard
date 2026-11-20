#pragma once

#include "core/policy.hpp"
#include "core/scanner.hpp"
#include "core/text_index.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace dataguard::detail {

void runRule(const std::string& input, const TextIndex& index, const Policy& policy,
             const std::string& id, const std::string& category, const std::string& severity,
             const std::string& description, const std::string& pattern, bool caseInsensitive,
             bool (*validator)(std::string_view), std::vector<Finding>& out);

void scanEntropyBearers(const std::string& input, const TextIndex& index, const Policy& policy,
                        std::vector<Finding>& out);

std::vector<Finding> deduplicateFindings(std::vector<Finding>& raw);
int computeRiskScore(const std::vector<Finding>& findings, bool& hasCritical);

}  // namespace dataguard::detail

