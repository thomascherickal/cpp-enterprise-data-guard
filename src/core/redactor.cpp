#include "core/redactor.hpp"

#include "core/scanner.hpp"

namespace dataguard {

std::string Redactor::apply(const std::string& input,
                            const std::vector<Finding>& findings,
                            const std::string& mode) {
    std::string out;
    out.reserve(input.size());
    std::size_t cursor = 0;
    for (const auto& f : findings) {
        if (f.start > cursor) {
            out.append(input, cursor, f.start - cursor);
        }
        if (mode == "REMOVE") {
            // Omit matched secret/PII
        } else if (mode == "MASK") {
            out.append(f.maskedMatch);
        } else {
            out.append("[REDACTED:");
            out.append(f.ruleId);
            out.push_back(']');
        }
        cursor = f.end;
    }
    if (cursor < input.size()) {
        out.append(input, cursor, input.size() - cursor);
    }
    return out;
}

}  // namespace dataguard

