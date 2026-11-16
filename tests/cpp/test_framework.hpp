#pragma once

#include <cmath>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace dataguard::test {

struct TestCase {
    std::string name;
    std::function<void()> fn;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

inline int& failureCount() {
    static int failures = 0;
    return failures;
}

struct Registrar {
    Registrar(const char* name, std::function<void()> fn) {
        registry().push_back({name, std::move(fn)});
    }
};

#define DG_CONCAT_INNER(a, b) a##b
#define DG_CONCAT(a, b) DG_CONCAT_INNER(a, b)

#define TEST_CASE(name)                                                   \
    static void DG_CONCAT(dg_test_fn_, __LINE__)();                       \
    static dataguard::test::Registrar DG_CONCAT(dg_test_reg_, __LINE__)(  \
        name, DG_CONCAT(dg_test_fn_, __LINE__));                          \
    static void DG_CONCAT(dg_test_fn_, __LINE__)()

#define CHECK(expr)                                                              \
    do {                                                                         \
        if (!(expr)) {                                                           \
            ++dataguard::test::failureCount();                                   \
            std::cerr << "[FAIL] " << __FILE__ << ":" << __LINE__ << " CHECK("   \
                      << #expr << ")\n";                                         \
        }                                                                        \
    } while (false)

#define CHECK_EQ(a, b)                                                             \
    do {                                                                           \
        auto _va = (a);                                                            \
        auto _vb = (b);                                                            \
        if (!(_va == _vb)) {                                                       \
            ++dataguard::test::failureCount();                                     \
            std::cerr << "[FAIL] " << __FILE__ << ":" << __LINE__ << " CHECK_EQ("  \
                      << #a << ", " << #b << ") got [" << _va << "] vs [" << _vb   \
                      << "]\n";                                                    \
        }                                                                          \
    } while (false)

#define CHECK_NEAR(a, b, eps)                                                       \
    do {                                                                            \
        double _va = (a);                                                           \
        double _vb = (b);                                                           \
        if (std::fabs(_va - _vb) > (eps)) {                                         \
            ++dataguard::test::failureCount();                                      \
            std::cerr << "[FAIL] " << __FILE__ << ":" << __LINE__ << " CHECK_NEAR(" \
                      << #a << ", " << #b << ") got [" << _va << "] vs [" << _vb    \
                      << "]\n";                                                     \
        }                                                                           \
    } while (false)

}  // namespace dataguard::test

