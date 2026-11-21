#include "core/scanner_engine.hpp"

#include "core/json.hpp"

#include <algorithm>
#include <utility>

namespace dataguard::detail {

std::vector<Finding> deduplicateFindings(std::vector<Finding> &raw) {
  std::sort(raw.begin(), raw.end(), [](const Finding &a, const Finding &b) {
    if (a.start != b.start)
      return a.start < b.start;
    int ra = severityRank(a.severity);
    int rb = severityRank(b.severity);
    if (ra != rb)
      return ra > rb;
    return (a.end - a.start) > (b.end - b.start);
  });

  std::vector<Finding> unique;
  std::size_t lastEnd = 0;
  for (auto &f : raw) {
    if (f.start < lastEnd)
      continue;
    lastEnd = f.end;
    unique.push_back(std::move(f));
  }
  return unique;
}

int computeRiskScore(const std::vector<Finding> &findings, bool &hasCritical) {
  int score = 0;
  hasCritical = false;
  for (const auto &f : findings) {
    int r = severityRank(f.severity);
    if (r == 4) {
      score += 40;
      hasCritical = true;
    } else if (r == 3)
      score += 25;
    else if (r == 2)
      score += 15;
    else
      score += 5;
  }
  return std::min(100, score);
}

} // namespace dataguard::detail

namespace dataguard {

std::string ScanResult::toJson(int indent) const {
  auto root = json::Value::makeObject();
  root.set("blocked", json::Value(blocked));
  root.set("riskScore", json::Value(riskScore));
  root.set("findingCount", json::Value(static_cast<int>(findings.size())));
  root.set("redactedText", json::Value(redactedText));

  auto arr = json::Value::makeArray();
  for (const auto &f : findings) {
    auto item = json::Value::makeObject();
    item.set("ruleId", json::Value(f.ruleId));
    item.set("category", json::Value(f.category));
    item.set("severity", json::Value(f.severity));
    item.set("description", json::Value(f.description));
    item.set("maskedMatch", json::Value(f.maskedMatch));
    item.set("start", json::Value(static_cast<int>(f.start)));
    item.set("end", json::Value(static_cast<int>(f.end)));
    item.set("line", json::Value(static_cast<int>(f.line)));
    item.set("column", json::Value(static_cast<int>(f.column)));
    item.set("entropy", json::Value(f.entropy));
    arr.push(std::move(item));
  }
  root.set("findings", std::move(arr));
  return root.dump(indent);
}

} // namespace dataguard
