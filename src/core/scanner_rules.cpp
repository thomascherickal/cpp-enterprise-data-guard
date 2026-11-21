#include "core/scanner_engine.hpp"

#include "core/validators.hpp"

#include <regex>
#include <utility>

namespace dataguard::detail {

namespace {

bool isAllowlisted(const std::vector<std::string> &allowlist,
                   const std::string &match) {
  for (const auto &entry : allowlist) {
    if (!entry.empty() && match.find(entry) != std::string::npos)
      return true;
  }
  return false;
}

} // namespace

void runRule(const std::string &input, const TextIndex &index,
             const Policy &policy, const std::string &id,
             const std::string &category, const std::string &severity,
             const std::string &description, const std::string &pattern,
             bool caseInsensitive, bool (*validator)(std::string_view),
             std::vector<Finding> &out) {
  if (severityRank(severity) < severityRank(policy.minSeverity))
    return;
  auto flags = std::regex::ECMAScript;
  if (caseInsensitive)
    flags |= std::regex::icase;
  std::regex re(pattern, flags);

  auto begin = std::sregex_iterator(input.begin(), input.end(), re);
  auto end = std::sregex_iterator();
  for (auto it = begin; it != end; ++it) {
    const auto &m = *it;
    std::string matched = m.str(0);
    if (matched.empty() || isAllowlisted(policy.allowlist, matched))
      continue;
    if (validator != nullptr && !validator(matched))
      continue;

    std::string entropyTarget =
        (m.size() > 1 && m[1].matched) ? m.str(1) : matched;
    Finding f;
    f.ruleId = id;
    f.category = category;
    f.severity = severity;
    f.description = description;
    f.maskedMatch = validators::maskSecret(matched);
    f.start = static_cast<std::size_t>(m.position(0));
    f.end = f.start + static_cast<std::size_t>(m.length(0));
    index.locate(f.start, f.line, f.column);
    f.entropy = validators::shannonEntropy(entropyTarget);
    out.push_back(std::move(f));
  }
}

void scanEntropyBearers(const std::string &input, const TextIndex &index,
                        const Policy &policy, std::vector<Finding> &out) {
  if (!policy.enableEntropy ||
      severityRank("HIGH") < severityRank(policy.minSeverity))
    return;
  static const std::regex kBearerRe(
      R"(Authorization\s*:\s*Bearer\s+([A-Za-z0-9._~+/-]{20,}=*))",
      std::regex::ECMAScript | std::regex::icase);
  auto begin = std::sregex_iterator(input.begin(), input.end(), kBearerRe);
  auto end = std::sregex_iterator();
  for (auto it = begin; it != end; ++it) {
    const auto &m = *it;
    std::string token = m.str(1);
    double ent = validators::shannonEntropy(token);
    if (ent >= policy.entropyThreshold &&
        !isAllowlisted(policy.allowlist, token)) {
      Finding f;
      f.ruleId = "SEC-HIGH-ENTROPY-BEARER";
      f.category = "SECRET";
      f.severity = "HIGH";
      f.description = "High-Entropy Bearer Token";
      f.maskedMatch = validators::maskSecret(m.str(0));
      f.start = static_cast<std::size_t>(m.position(0));
      f.end = f.start + static_cast<std::size_t>(m.length(0));
      index.locate(f.start, f.line, f.column);
      f.entropy = ent;
      out.push_back(std::move(f));
    }
  }
}

} // namespace dataguard::detail
