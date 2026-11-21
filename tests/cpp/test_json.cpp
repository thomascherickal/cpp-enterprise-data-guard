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

  Value nullCStr(static_cast<const char *>(nullptr));
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

  Value x(5);
  x.mutableArray().push_back(Value(1));
  CHECK(x.isArray());
  x.mutableObject()["a"] = Value(2);
  CHECK(x.isObject());
}
