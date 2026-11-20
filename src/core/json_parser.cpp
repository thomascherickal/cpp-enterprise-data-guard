#include "core/json_parser.hpp"

#include <cctype>
#include <utility>

namespace dataguard::json::detail {

ParseResult Parser::parseRoot() {
    skipWs();
    Value val;
    std::string err;
    if (!parseValue(val, err)) return {false, err, Value{}};
    skipWs();
    if (pos_ != src_.size()) return {false, "Unexpected trailing characters", Value{}};
    return {true, "", std::move(val)};
}

void Parser::skipWs() {
    while (pos_ < src_.size() &&
           (src_[pos_] == ' ' || src_[pos_] == '\t' || src_[pos_] == '\n' || src_[pos_] == '\r')) {
        ++pos_;
    }
}

bool Parser::parseValue(Value& out, std::string& err) {
    skipWs();
    if (pos_ >= src_.size()) { err = "Unexpected end of JSON input"; return false; }
    char c = src_[pos_];
    if (c == 'n') return parseLiteral("null", Value{}, out, err);
    if (c == 't') return parseLiteral("true", Value{true}, out, err);
    if (c == 'f') return parseLiteral("false", Value{false}, out, err);
    if (c == '"') {
        std::string s;
        if (!parseString(s, err)) return false;
        out = Value(std::move(s));
        return true;
    }
    if (c == '[') return parseArray(out, err);
    if (c == '{') return parseObject(out, err);
    if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return parseNumber(out, err);
    err = std::string("Unexpected character '") + c + "' at position " + std::to_string(pos_);
    return false;
}

bool Parser::parseLiteral(const char* lit, Value val, Value& out, std::string& err) {
    std::size_t len = 0;
    while (lit[len] != '\0') ++len;
    if (src_.compare(pos_, len, lit) != 0) {
        err = "Invalid literal at position " + std::to_string(pos_);
        return false;
    }
    pos_ += len;
    out = std::move(val);
    return true;
}

bool Parser::parseArray(Value& out, std::string& err) {
    ++pos_;
    Value::Array arr;
    skipWs();
    if (pos_ < src_.size() && src_[pos_] == ']') { ++pos_; out = Value(std::move(arr)); return true; }
    while (true) {
        Value elem;
        if (!parseValue(elem, err)) return false;
        arr.push_back(std::move(elem));
        skipWs();
        if (pos_ >= src_.size()) { err = "Unterminated array"; return false; }
        if (src_[pos_] == ']') { ++pos_; out = Value(std::move(arr)); return true; }
        if (src_[pos_] != ',') { err = "Expected ',' or ']' in array"; return false; }
        ++pos_;
    }
}

bool Parser::parseObject(Value& out, std::string& err) {
    ++pos_;
    Value::Object obj;
    skipWs();
    if (pos_ < src_.size() && src_[pos_] == '}') { ++pos_; out = Value(std::move(obj)); return true; }
    while (true) {
        skipWs();
        if (pos_ >= src_.size() || src_[pos_] != '"') { err = "Expected string key"; return false; }
        std::string key;
        if (!parseString(key, err)) return false;
        skipWs();
        if (pos_ >= src_.size() || src_[pos_] != ':') { err = "Expected ':' after key"; return false; }
        ++pos_;
        Value val;
        if (!parseValue(val, err)) return false;
        obj[std::move(key)] = std::move(val);
        skipWs();
        if (pos_ >= src_.size()) { err = "Unterminated object"; return false; }
        if (src_[pos_] == '}') { ++pos_; out = Value(std::move(obj)); return true; }
        if (src_[pos_] != ',') { err = "Expected ',' or '}' in object"; return false; }
        ++pos_;
    }
}

}  // namespace dataguard::json::detail

