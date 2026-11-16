#include "core/text_index.hpp"

#include <algorithm>

namespace dataguard {

TextIndex::TextIndex(const std::string& text) : lineStarts_{0} {
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\n') {
            lineStarts_.push_back(i + 1);
        }
    }
}

void TextIndex::locate(std::size_t offset, std::size_t& outLine, std::size_t& outColumn) const {
    auto it = std::upper_bound(lineStarts_.begin(), lineStarts_.end(), offset);
    std::size_t lineIdx = static_cast<std::size_t>(it - lineStarts_.begin()) - 1;
    outLine = lineIdx + 1;
    outColumn = (offset - lineStarts_[lineIdx]) + 1;
}

}  // namespace dataguard

