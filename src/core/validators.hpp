#pragma once

#include <string>
#include <string_view>

namespace dataguard::validators {

double shannonEntropy(std::string_view s);
bool isValidLuhn(std::string_view input);
bool isValidIban(std::string_view input);
bool isValidSsn(std::string_view input);
std::string maskSecret(std::string_view value, std::size_t keepStart = 4,
                       std::size_t keepEnd = 4);

}  // namespace dataguard::validators

