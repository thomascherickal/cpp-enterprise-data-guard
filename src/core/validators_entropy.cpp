#include "core/validators.hpp"

#include <array>
#include <cmath>
#include <string>

namespace dataguard::validators {

double shannonEntropy(std::string_view s) {
    if (s.empty()) return 0.0;
    std::array<std::size_t, 256> counts{};
    for (unsigned char c : s) {
        counts[c]++;
    }
    double entropy = 0.0;
    const double len = static_cast<double>(s.size());
    for (std::size_t count : counts) {
        if (count == 0) continue;
        double p = static_cast<double>(count) / len;
        entropy -= p * std::log2(p);
    }
    return entropy;
}

std::string maskSecret(std::string_view value, std::size_t keepStart, std::size_t keepEnd) {
    if (value.size() <= keepStart + keepEnd + 2) {
        return "****";
    }
    std::string out;
    out.append(value.substr(0, keepStart));
    out.append("****");
    out.append(value.substr(value.size() - keepEnd));
    return out;
}

}  // namespace dataguard::validators

