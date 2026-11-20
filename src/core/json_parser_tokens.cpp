#include "core/json_parser.hpp"

#include <cctype>
#include <cstdlib>

namespace dataguard::json::detail {

bool Parser::parseUnicodeEscape(std::string& out, std::string& err) {
    if (pos_ + 4 > src_.size()) { err = "Incomplete \\uXXXX escape"; return false; }
    unsigned int code = 0;
    for (int i = 0; i < 4; ++i) {
        char h = src_[pos_++];
        code <<= 4;
        if (h >= '0' && h <= '9') code |= static_cast<unsigned int>(h - '0');
        else if (h >= 'a' && h <= 'f') code |= static_cast<unsigned int>(h - 'a' + 10);
        else if (h >= 'A' && h <= 'F') code |= static_cast<unsigned int>(h - 'A' + 10);
        else { err = "Invalid hex digit in \\uXXXX escape"; return false; }
    }
    if (code <= 0x7F) {
        out.push_back(static_cast<char>(code));
    } else if (code <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | ((code >> 6) & 0x1F)));
        out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xE0 | ((code >> 12) & 0x0F)));
        out.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
    }
    return true;
}

bool Parser::parseString(std::string& out, std::string& err) {
    ++pos_;
    out.clear();
    while (pos_ < src_.size()) {
        char c = src_[pos_++];
        if (c == '"') return true;
        if (c != '\\') { out.push_back(c); continue; }
        if (pos_ >= src_.size()) { err = "Unterminated escape sequence"; return false; }
        char esc = src_[pos_++];
        switch (esc) {
            case '"': out.push_back('"'); break;
            case '\\': out.push_back('\\'); break;
            case '/': out.push_back('/'); break;
            case 'b': out.push_back('\b'); break;
            case 'f': out.push_back('\f'); break;
            case 'n': out.push_back('\n'); break;
            case 'r': out.push_back('\r'); break;
            case 't': out.push_back('\t'); break;
            case 'u': if (!parseUnicodeEscape(out, err)) return false; break;
            default: err = std::string("Unknown escape '\\") + esc + "'"; return false;
        }
    }
    err = "Unterminated string";
    return false;
}

bool Parser::parseNumber(Value& out, std::string& err) {
    std::size_t start = pos_;
    if (src_[pos_] == '-') ++pos_;
    if (pos_ >= src_.size() || !std::isdigit(static_cast<unsigned char>(src_[pos_]))) {
        err = "Invalid number"; return false;
    }
    while (pos_ < src_.size() && std::isdigit(static_cast<unsigned char>(src_[pos_]))) ++pos_;
    if (pos_ < src_.size() && src_[pos_] == '.') {
        ++pos_;
        if (pos_ >= src_.size() || !std::isdigit(static_cast<unsigned char>(src_[pos_]))) {
            err = "Invalid decimal fraction"; return false;
        }
        while (pos_ < src_.size() && std::isdigit(static_cast<unsigned char>(src_[pos_]))) ++pos_;
    }
    if (pos_ < src_.size() && (src_[pos_] == 'e' || src_[pos_] == 'E')) {
        ++pos_;
        if (pos_ < src_.size() && (src_[pos_] == '+' || src_[pos_] == '-')) ++pos_;
        if (pos_ >= src_.size() || !std::isdigit(static_cast<unsigned char>(src_[pos_]))) {
            err = "Invalid exponent"; return false;
        }
        while (pos_ < src_.size() && std::isdigit(static_cast<unsigned char>(src_[pos_]))) ++pos_;
    }
    char* endPtr = nullptr;
    out = Value(std::strtod(src_.substr(start, pos_ - start).c_str(), &endPtr));
    return true;
}

}  // namespace dataguard::json::detail

namespace dataguard::json {

ParseResult parse(const std::string& input) {
    detail::Parser parser(input);
    return parser.parseRoot();
}

}  // namespace dataguard::json

