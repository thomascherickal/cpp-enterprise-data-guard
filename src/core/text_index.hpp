#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace dataguard {

class TextIndex {
public:
    explicit TextIndex(const std::string& text);

    void locate(std::size_t offset, std::size_t& outLine, std::size_t& outColumn) const;

private:
    std::vector<std::size_t> lineStarts_;
};

}  // namespace dataguard

