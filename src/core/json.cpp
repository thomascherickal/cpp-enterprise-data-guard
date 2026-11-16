#include "core/json.hpp"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <utility>

namespace dataguard::json {

namespace {

const Value kNullValue;
const std::string kEmptyString;
const Value::Array kEmptyArray;
const Value::Object kEmptyObject;

class Parser {
public:
    explicit Parser(const std::string& src) : src_(src), pos_(0) {}

    ParseResult parseRoot() {
        skipWs();
        Value val;
        std::string err;
        if (!parseValue(val, err)) {
            return {false, err, Value{}};
        }
        skipWs();
        if (pos_ != src_.size()) {
            return {false, "Unexpected trailing characters at position " + std::to_string(pos_),
                    Value{}};
        }
        return {true, "", std::move(val)};
    }

private:
    const std::string& src_;
    std::size_t pos_;

    void skipWs() {
        while (pos_ < src_.size() &&
               (src_[pos_] == ' ' || src_[pos_] == '\t' || src_[pos_] == '\n' ||
                src_[pos_] == '\r')) {
            ++pos_;
        }
    }

    bool parseValue(Value& out, std::string& err) {
        skipWs();
        if (pos_ >= src_.size()) {
            err = "Unexpected end of JSON input";
            return false;
        }
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
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) {
            return parseNumber(out, err);
        }
        err = std::string("Unexpected character '") + c + "' at position " + std::to_string(pos_);
        return false;
    }

    bool parseLiteral(const char* lit, Value val, Value& out, std::string& err) {
        std::size_t len = 0;
        while (lit[len] != '\0') ++len;
        if (src_.compare(pos_, len, lit) != 0) {
            err = std::string("Invalid literal at position ") + std::to_string(pos_);
            return false;
        }
        pos_ += len;
        out = std::move(val);
        return true;
    }

    bool parseString(std::string& out, std::string& err) {
        ++pos_;  // consume opening quote
        out.clear();
        while (pos_ < src_.size()) {
            char c = src_[pos_++];
            if (c == '"') return true;
            if (c == '\\') {
                if (pos_ >= src_.size()) {
                    err = "Unterminated escape sequence";
                    return false;
                }
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
                    case 'u': {
                        if (pos_ + 4 > src_.size()) {
                            err = "Incomplete \\uXXXX escape";
                            return false;
                        }
                        unsigned int code = 0;
                        for (int i = 0; i < 4; ++i) {
                            char h = src_[pos_++];
                            code <<= 4;
                            if (h >= '0' && h <= '9') code |= static_cast<unsigned int>(h - '0');
                            else if (h >= 'a' && h <= 'f') code |= static_cast<unsigned int>(h - 'a' + 10);
                            else if (h >= 'A' && h <= 'F') code |= static_cast<unsigned int>(h - 'A' + 10);
                            else {
                                err = "Invalid hex digit in \\uXXXX escape";
                                return false;
                            }
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
                        break;
                    }
                    default:
                        err = std::string("Unknown escape '\\") + esc + "'";
                        return false;
                }
            } else {
                out.push_back(c);
            }
        }
        err = "Unterminated string";
        return false;
    }

    bool parseNumber(Value& out, std::string& err) {
        std::size_t start = pos_;
        if (src_[pos_] == '-') ++pos_;
        if (pos_ >= src_.size() || !std::isdigit(static_cast<unsigned char>(src_[pos_]))) {
            err = "Invalid number at position " + std::to_string(start);
            return false;
        }
        while (pos_ < src_.size() && std::isdigit(static_cast<unsigned char>(src_[pos_]))) ++pos_;
        if (pos_ < src_.size() && src_[pos_] == '.') {
            ++pos_;
            if (pos_ >= src_.size() || !std::isdigit(static_cast<unsigned char>(src_[pos_]))) {
                err = "Invalid decimal fraction at position " + std::to_string(start);
                return false;
            }
            while (pos_ < src_.size() && std::isdigit(static_cast<unsigned char>(src_[pos_]))) {
                ++pos_;
            }
        }
        if (pos_ < src_.size() && (src_[pos_] == 'e' || src_[pos_] == 'E')) {
            ++pos_;
            if (pos_ < src_.size() && (src_[pos_] == '+' || src_[pos_] == '-')) ++pos_;
            if (pos_ >= src_.size() || !std::isdigit(static_cast<unsigned char>(src_[pos_]))) {
                err = "Invalid exponent at position " + std::to_string(start);
                return false;
            }
            while (pos_ < src_.size() && std::isdigit(static_cast<unsigned char>(src_[pos_]))) {
                ++pos_;
            }
        }
        std::string numStr = src_.substr(start, pos_ - start);
        char* endPtr = nullptr;
        double val = std::strtod(numStr.c_str(), &endPtr);
        out = Value(val);
        return true;
    }

    bool parseArray(Value& out, std::string& err) {
        ++pos_;  // consume '['
        Value::Array arr;
        skipWs();
        if (pos_ < src_.size() && src_[pos_] == ']') {
            ++pos_;
            out = Value(std::move(arr));
            return true;
        }
        while (true) {
            Value elem;
            if (!parseValue(elem, err)) return false;
            arr.push_back(std::move(elem));
            skipWs();
            if (pos_ >= src_.size()) {
                err = "Unterminated array";
                return false;
            }
            if (src_[pos_] == ']') {
                ++pos_;
                out = Value(std::move(arr));
                return true;
            }
            if (src_[pos_] != ',') {
                err = "Expected ',' or ']' in array at position " + std::to_string(pos_);
                return false;
            }
            ++pos_;
        }
    }

    bool parseObject(Value& out, std::string& err) {
        ++pos_;  // consume '{'
        Value::Object obj;
        skipWs();
        if (pos_ < src_.size() && src_[pos_] == '}') {
            ++pos_;
            out = Value(std::move(obj));
            return true;
        }
        while (true) {
            skipWs();
            if (pos_ >= src_.size() || src_[pos_] != '"') {
                err = "Expected string key in object at position " + std::to_string(pos_);
                return false;
            }
            std::string key;
            if (!parseString(key, err)) return false;
            skipWs();
            if (pos_ >= src_.size() || src_[pos_] != ':') {
                err = "Expected ':' after key at position " + std::to_string(pos_);
                return false;
            }
            ++pos_;
            Value val;
            if (!parseValue(val, err)) return false;
            obj[std::move(key)] = std::move(val);
            skipWs();
            if (pos_ >= src_.size()) {
                err = "Unterminated object";
                return false;
            }
            if (src_[pos_] == '}') {
                ++pos_;
                out = Value(std::move(obj));
                return true;
            }
            if (src_[pos_] != ',') {
                err = "Expected ',' or '}' in object at position " + std::to_string(pos_);
                return false;
            }
            ++pos_;
        }
    }
};

}  // namespace

Value::Value(bool b) : type_(Type::Bool), boolVal_(b) {}
Value::Value(double n) : type_(Type::Number), numVal_(n) {}
Value::Value(int n) : type_(Type::Number), numVal_(static_cast<double>(n)) {}
Value::Value(const char* s) : type_(Type::String), strVal_(s ? s : "") {}
Value::Value(std::string s) : type_(Type::String), strVal_(std::move(s)) {}
Value::Value(Array arr) : type_(Type::Array), arrVal_(std::move(arr)) {}
Value::Value(Object obj) : type_(Type::Object), objVal_(std::move(obj)) {}

Value Value::makeArray() { return Value(Array{}); }
Value Value::makeObject() { return Value(Object{}); }

bool Value::asBool(bool fallback) const {
    return isBool() ? boolVal_ : fallback;
}

double Value::asNumber(double fallback) const {
    return isNumber() ? numVal_ : fallback;
}

int Value::asInt(int fallback) const {
    return isNumber() ? static_cast<int>(numVal_) : fallback;
}

const std::string& Value::asString() const {
    return isString() ? strVal_ : kEmptyString;
}

const Value::Array& Value::asArray() const {
    return isArray() ? arrVal_ : kEmptyArray;
}

const Value::Object& Value::asObject() const {
    return isObject() ? objVal_ : kEmptyObject;
}

Value::Array& Value::mutableArray() {
    if (!isArray()) {
        type_ = Type::Array;
        arrVal_.clear();
    }
    return arrVal_;
}

Value::Object& Value::mutableObject() {
    if (!isObject()) {
        type_ = Type::Object;
        objVal_.clear();
    }
    return objVal_;
}

bool Value::contains(const std::string& key) const {
    if (!isObject()) return false;
    return objVal_.find(key) != objVal_.end();
}

const Value& Value::get(const std::string& key) const {
    if (!isObject()) return kNullValue;
    auto it = objVal_.find(key);
    return it != objVal_.end() ? it->second : kNullValue;
}

void Value::set(const std::string& key, Value val) {
    mutableObject()[key] = std::move(val);
}

void Value::push(Value val) {
    mutableArray().push_back(std::move(val));
}

std::string escapeString(const std::string& s) {
    std::string out = "\"";
    for (unsigned char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[7];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out.push_back(static_cast<char>(c));
                }
                break;
        }
    }
    out.push_back('"');
    return out;
}

std::string Value::dump(int indent) const {
    std::string out;
    dumpImpl(out, indent, 0);
    return out;
}

void Value::dumpImpl(std::string& out, int indent, int depth) const {
    switch (type_) {
        case Type::Null:
            out += "null";
            break;
        case Type::Bool:
            out += (boolVal_ ? "true" : "false");
            break;
        case Type::Number: {
            if (std::floor(numVal_) == numVal_ && std::abs(numVal_) < 1e15) {
                std::ostringstream oss;
                oss << static_cast<long long>(numVal_);
                out += oss.str();
            } else {
                std::ostringstream oss;
                oss << numVal_;
                out += oss.str();
            }
            break;
        }
        case Type::String:
            out += escapeString(strVal_);
            break;
        case Type::Array: {
            if (arrVal_.empty()) {
                out += "[]";
                break;
            }
            out.push_back('[');
            for (std::size_t i = 0; i < arrVal_.size(); ++i) {
                if (i > 0) out.push_back(',');
                if (indent >= 0) {
                    out.push_back('\n');
                    out.append(static_cast<std::size_t>((depth + 1) * indent), ' ');
                }
                arrVal_[i].dumpImpl(out, indent, depth + 1);
            }
            if (indent >= 0) {
                out.push_back('\n');
                out.append(static_cast<std::size_t>(depth * indent), ' ');
            }
            out.push_back(']');
            break;
        }
        case Type::Object: {
            if (objVal_.empty()) {
                out += "{}";
                break;
            }
            out.push_back('{');
            std::size_t idx = 0;
            for (const auto& kv : objVal_) {
                if (idx++ > 0) out.push_back(',');
                if (indent >= 0) {
                    out.push_back('\n');
                    out.append(static_cast<std::size_t>((depth + 1) * indent), ' ');
                }
                out += escapeString(kv.first);
                out += (indent >= 0 ? ": " : ":");
                kv.second.dumpImpl(out, indent, depth + 1);
            }
            if (indent >= 0) {
                out.push_back('\n');
                out.append(static_cast<std::size_t>(depth * indent), ' ');
            }
            out.push_back('}');
            break;
        }
    }
}

ParseResult parse(const std::string& input) {
    Parser parser(input);
    return parser.parseRoot();
}

}  // namespace dataguard::json

