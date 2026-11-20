#include "core/json.hpp"

#include <cmath>
#include <cstdio>
#include <sstream>

namespace dataguard::json {

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
        case Type::Null: out += "null"; break;
        case Type::Bool: out += (boolVal_ ? "true" : "false"); break;
        case Type::Number: {
            std::ostringstream oss;
            if (std::floor(numVal_) == numVal_ && std::abs(numVal_) < 1e15) {
                oss << static_cast<long long>(numVal_);
            } else {
                oss << numVal_;
            }
            out += oss.str();
            break;
        }
        case Type::String: out += escapeString(strVal_); break;
        case Type::Array: {
            if (arrVal_.empty()) { out += "[]"; break; }
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
            if (objVal_.empty()) { out += "{}"; break; }
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

}  // namespace dataguard::json

