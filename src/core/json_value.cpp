#include "core/json.hpp"

#include <utility>

namespace dataguard::json {

namespace {
const Value kNullValue;
const std::string kEmptyString;
const Value::Array kEmptyArray;
const Value::Object kEmptyObject;
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

bool Value::asBool(bool fallback) const { return isBool() ? boolVal_ : fallback; }
double Value::asNumber(double fallback) const { return isNumber() ? numVal_ : fallback; }
int Value::asInt(int fallback) const { return isNumber() ? static_cast<int>(numVal_) : fallback; }
const std::string& Value::asString() const { return isString() ? strVal_ : kEmptyString; }
const Value::Array& Value::asArray() const { return isArray() ? arrVal_ : kEmptyArray; }
const Value::Object& Value::asObject() const { return isObject() ? objVal_ : kEmptyObject; }

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
    return isObject() && objVal_.find(key) != objVal_.end();
}

const Value& Value::get(const std::string& key) const {
    if (!isObject()) return kNullValue;
    auto it = objVal_.find(key);
    return it != objVal_.end() ? it->second : kNullValue;
}

void Value::set(const std::string& key, Value val) { mutableObject()[key] = std::move(val); }
void Value::push(Value val) { mutableArray().push_back(std::move(val)); }

}  // namespace dataguard::json

