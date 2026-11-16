#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace dataguard::json {

enum class Type { Null, Bool, Number, String, Array, Object };

class Value {
public:
    using Array = std::vector<Value>;
    using Object = std::map<std::string, Value>;

    Value() = default;
    explicit Value(bool b);
    explicit Value(double n);
    explicit Value(int n);
    explicit Value(const char* s);
    explicit Value(std::string s);
    explicit Value(Array arr);
    explicit Value(Object obj);

    static Value makeArray();
    static Value makeObject();

    Type type() const { return type_; }
    bool isNull() const { return type_ == Type::Null; }
    bool isBool() const { return type_ == Type::Bool; }
    bool isNumber() const { return type_ == Type::Number; }
    bool isString() const { return type_ == Type::String; }
    bool isArray() const { return type_ == Type::Array; }
    bool isObject() const { return type_ == Type::Object; }

    bool asBool(bool fallback = false) const;
    double asNumber(double fallback = 0.0) const;
    int asInt(int fallback = 0) const;
    const std::string& asString() const;
    const Array& asArray() const;
    const Object& asObject() const;

    Array& mutableArray();
    Object& mutableObject();

    bool contains(const std::string& key) const;
    const Value& get(const std::string& key) const;
    void set(const std::string& key, Value val);
    void push(Value val);

    std::string dump(int indent = -1) const;

private:
    Type type_ = Type::Null;
    bool boolVal_ = false;
    double numVal_ = 0.0;
    std::string strVal_;
    Array arrVal_;
    Object objVal_;

    void dumpImpl(std::string& out, int indent, int depth) const;
};

struct ParseResult {
    bool ok = false;
    std::string error;
    Value value;
};

ParseResult parse(const std::string& input);
std::string escapeString(const std::string& s);

}  // namespace dataguard::json

