#pragma once

#include "core/json.hpp"

#include <cstddef>
#include <string>

namespace dataguard::json::detail {

class Parser {
public:
    explicit Parser(const std::string& src) : src_(src), pos_(0) {}
    ParseResult parseRoot();

private:
    const std::string& src_;
    std::size_t pos_;

    void skipWs();
    bool parseValue(Value& out, std::string& err);
    bool parseLiteral(const char* lit, Value val, Value& out, std::string& err);
    bool parseString(std::string& out, std::string& err);
    bool parseUnicodeEscape(std::string& out, std::string& err);
    bool parseNumber(Value& out, std::string& err);
    bool parseArray(Value& out, std::string& err);
    bool parseObject(Value& out, std::string& err);
};

}  // namespace dataguard::json::detail

