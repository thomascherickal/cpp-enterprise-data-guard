#include "core/validators.hpp"

#include <cctype>
#include <string>

namespace dataguard::validators {

bool isValidLuhn(std::string_view input) {
    std::string digits;
    digits.reserve(input.size());
    for (char c : input) {
        if (c == ' ' || c == '-') continue;
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
        digits.push_back(c);
    }
    if (digits.size() < 13 || digits.size() > 19) return false;

    bool allSame = true;
    for (std::size_t i = 1; i < digits.size(); ++i) {
        if (digits[i] != digits[0]) { allSame = false; break; }
    }
    if (allSame) return false;

    int sum = 0;
    bool alt = false;
    for (std::size_t i = digits.size(); i > 0; --i) {
        int d = digits[i - 1] - '0';
        if (alt) {
            d *= 2;
            if (d > 9) d -= 9;
        }
        sum += d;
        alt = !alt;
    }
    return (sum % 10) == 0;
}

bool isValidIban(std::string_view input) {
    std::string clean;
    clean.reserve(input.size());
    for (char c : input) {
        if (c == ' ' || c == '-') continue;
        if (!std::isalnum(static_cast<unsigned char>(c))) return false;
        clean.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    }
    if (clean.size() < 15 || clean.size() > 34) return false;
    if (!std::isalpha(static_cast<unsigned char>(clean[0])) ||
        !std::isalpha(static_cast<unsigned char>(clean[1])) ||
        !std::isdigit(static_cast<unsigned char>(clean[2])) ||
        !std::isdigit(static_cast<unsigned char>(clean[3]))) {
        return false;
    }

    std::string rearranged = clean.substr(4) + clean.substr(0, 4);
    int mod = 0;
    for (char c : rearranged) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            mod = (mod * 10 + (c - '0')) % 97;
        } else {
            mod = (mod * 100 + (c - 'A') + 10) % 97;
        }
    }
    return mod == 1;
}

bool isValidSsn(std::string_view input) {
    if (input.size() != 11 || input[3] != '-' || input[6] != '-') return false;
    for (std::size_t i = 0; i < input.size(); ++i) {
        if (i == 3 || i == 6) continue;
        if (!std::isdigit(static_cast<unsigned char>(input[i]))) return false;
    }
    int area = (input[0] - '0') * 100 + (input[1] - '0') * 10 + (input[2] - '0');
    int group = (input[4] - '0') * 10 + (input[5] - '0');
    int serial = (input[7] - '0') * 1000 + (input[8] - '0') * 100 +
                 (input[9] - '0') * 10 + (input[10] - '0');
    return area != 0 && area != 666 && area < 900 && group != 0 && serial != 0;
}

}  // namespace dataguard::validators

