#include "core/json.hpp"
#include "test_framework.hpp"

using namespace dataguard::json;

TEST_CASE("json primitives and accessors") {
    Value n;
    CHECK(n.isNull());
    CHECK(!n.isBool());
    CHECK_EQ(n.asBool(true), true);
    CHECK_NEAR(n.asNumber(3.5), 3.5, 1e-9);
    CHECK_EQ(n.asInt(7), 7);
    CHECK_EQ(n.asString(), "");
    CHECK(n.asArray().empty());
    CHECK(n.asObject().empty());
    CHECK(!n.contains("k"));
    CHECK(n.get("k").isNull());

    Value b(true);
    CHECK(b.isBool());
    CHECK_EQ(b.asBool(false), true);

    Value num(42);
    CHECK(num.isNumber());
    CHECK_EQ(num.asInt(), 42);
    CHECK_NEAR(num.asNumber(), 42.0, 1e-9);

    Value dbl(3.25);
    CHECK_NEAR(dbl.asNumber(), 3.25, 1e-9);

    Value s("hello");
    CHECK(s.isString());
    CHECK_EQ(s.asString(), "hello");

    Value nullCStr(static_cast<const char*>(nullptr));
    CHECK_EQ(nullCStr.asString(), "");
}

TEST_CASE("json containers and dump") {
    auto arr = Value::makeArray();
    arr.push(Value(1));
    arr.push(Value("two"));
    arr.push(Value(false));
    arr.push(Value{});
    CHECK(arr.isArray());
    CHECK_EQ(arr.asArray().size(), 4u);

    auto obj = Value::makeObject();
    obj.set("arr", arr);
    obj.set("emptyArr", Value::makeArray());
    obj.set("emptyObj", Value::makeObject());
    obj.set("pi", Value(3.14));
    CHECK(obj.contains("arr"));
    CHECK(!obj.contains("missing"));
    CHECK(obj.get("missing").isNull());

    std::string compact = obj.dump(-1);
    std::string pretty = obj.dump(2);
    CHECK(compact.find("\"arr\":[1,\"two\",false,null]") != std::string::npos);
    CHECK(pretty.find('\n') != std::string::npos);

    // Mutating non-array/non-object resets type cleanly
    Value x(5);
    x.mutableArray().push_back(Value(1));
    CHECK(x.isArray());
    x.mutableObject()["a"] = Value(2);
    CHECK(x.isObject());
}

TEST_CASE("json parse valid inputs and escapes") {
    auto r1 = parse(" \t\n\r { \"a\": [true, false, null, -12, 3.5e+2, 1.25E-1], "
                    "\"s\": \"\\\"\\\\\\/\\b\\f\\n\\r\\t\\u0041\\u00e9\\u20ac\" } ");
    CHECK(r1.ok);
    CHECK(r1.value.isObject());
    const auto& arr = r1.value.get("a").asArray();
    CHECK_EQ(arr.size(), 6u);
    CHECK_EQ(arr[0].asBool(), true);
    CHECK_EQ(arr[1].asBool(), false);
    CHECK(arr[2].isNull());
    CHECK_EQ(arr[3].asInt(), -12);
    CHECK_NEAR(arr[4].asNumber(), 350.0, 1e-6);
    CHECK_NEAR(arr[5].asNumber(), 0.125, 1e-6);

    const std::string& decoded = r1.value.get("s").asString();
    CHECK(decoded.find('A') != std::string::npos);
    CHECK(decoded.find('\n') != std::string::npos);

    auto r2 = parse("[]");
    CHECK(r2.ok);
    CHECK(r2.value.asArray().empty());

    auto r3 = parse("{}");
    CHECK(r3.ok);
    CHECK(r3.value.asObject().empty());

    // Control char escaping in dump
    std::string ctrl;
    ctrl.push_back('\x01');
    ctrl += "\"\\\b\f\n\r\t";
    Value ctrlVal(ctrl);
    std::string dumped = ctrlVal.dump();
    CHECK(dumped.find("\\u0001") != std::string::npos);
}

TEST_CASE("json parse error paths") {
    CHECK(!parse("").ok);
    CHECK(!parse("   ").ok);
    CHECK(!parse("true 1").ok);
    CHECK(!parse("@").ok);
    CHECK(!parse("tru").ok);
    CHECK(!parse("\"unterminated").ok);
    CHECK(!parse("\"\\").ok);
    CHECK(!parse("\"\\x\"").ok);
    CHECK(!parse("\"\\u12\"").ok);
    CHECK(!parse("\"\\u12zz\"").ok);
    CHECK(!parse("\"\\u1234\"").ok == false);
    CHECK(!parse("-").ok);
    CHECK(!parse("1.").ok);
    CHECK(!parse("1e").ok);
    CHECK(!parse("1e+").ok);
    CHECK(!parse("[1, 2").ok);
    CHECK(!parse("[1 2]").ok);
    CHECK(!parse("{1: 2}").ok);
    CHECK(!parse("{\"a\" 2}").ok);
    CHECK(!parse("{\"a\": 2").ok);
    CHECK(!parse("{\"a\": 2 \"b\": 3}").ok);
    CHECK(!parse("[\"\\u0041\", \"\\u004F\", \"\\u004a\"]").ok == false);
}

