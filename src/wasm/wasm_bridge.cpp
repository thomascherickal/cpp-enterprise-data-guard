#include "core/json.hpp"
#include "core/policy.hpp"
#include "core/scanner.hpp"

#include <emscripten/bind.h>
#include <string>
#include <vector>

namespace {

std::string scanText(const std::string& text, const std::string& policyJson) {
    std::string err;
    dataguard::Policy policy = dataguard::Policy::fromJson(policyJson, err);
    if (!err.empty()) {
        auto errObj = dataguard::json::Value::makeObject();
        errObj.set("error", dataguard::json::Value(err));
        errObj.set("blocked", dataguard::json::Value(true));
        errObj.set("riskScore", dataguard::json::Value(100));
        errObj.set("findingCount", dataguard::json::Value(0));
        errObj.set("findings", dataguard::json::Value::makeArray());
        errObj.set("redactedText", dataguard::json::Value(""));
        return errObj.dump(0);
    }
    dataguard::Scanner scanner(policy);
    return scanner.scan(text).toJson(0);
}

std::string redactText(const std::string& text, const std::string& policyJson) {
    std::string err;
    dataguard::Policy policy = dataguard::Policy::fromJson(policyJson, err);
    if (!err.empty()) {
        return text;
    }
    dataguard::Scanner scanner(policy);
    return scanner.redact(text);
}

std::string getDefaultPolicy() {
    return dataguard::Policy::defaultEnterprise().toJson(2);
}

std::string validatePolicy(const std::string& policyJson) {
    auto out = dataguard::json::Value::makeObject();
    std::string parseErr;
    dataguard::Policy policy = dataguard::Policy::fromJson(policyJson, parseErr);
    auto errArr = dataguard::json::Value::makeArray();
    if (!parseErr.empty()) {
        errArr.push(dataguard::json::Value(parseErr));
        out.set("valid", dataguard::json::Value(false));
        out.set("errors", std::move(errArr));
        return out.dump(0);
    }
    std::vector<std::string> errors;
    bool valid = policy.validate(errors);
    for (const auto& e : errors) {
        errArr.push(dataguard::json::Value(e));
    }
    out.set("valid", dataguard::json::Value(valid));
    out.set("errors", std::move(errArr));
    return out.dump(0);
}

std::string getVersion() {
    return dataguard::Scanner::version();
}

}  // namespace

EMSCRIPTEN_BINDINGS(data_guard_module) {
    emscripten::function("scanText", &scanText);
    emscripten::function("redactText", &redactText);
    emscripten::function("getDefaultPolicy", &getDefaultPolicy);
    emscripten::function("validatePolicy", &validatePolicy);
    emscripten::function("getVersion", &getVersion);
}

