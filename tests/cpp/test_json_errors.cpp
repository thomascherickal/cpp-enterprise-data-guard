#include "core/json.hpp"
#include "test_framework.hpp"

using namespace dataguard::json;

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

    CHECK(parse("[]").ok);
    CHECK(parse("{}").ok);

    std::string ctrl;
    ctrl.push_back('\x01');
    ctrl += "\"\\\b\f\n\r\t";
    CHECK(Value(ctrl).dump().find("\\u0001") != std::string::npos);
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
    CHECK(parse("\"\\u1234\"").ok);
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
    CHECK(parse("[\"\\u0041\", \"\\u004F\", \"\\u004a\"]").ok);
}

