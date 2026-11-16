#pragma once

#include <string>
#include <vector>

namespace dataguard {

struct Finding;

class Redactor {
public:
    static std::string apply(const std::string& input,
                             const std::vector<Finding>& findings,
                             const std::string& mode);
};

}  // namespace dataguard

