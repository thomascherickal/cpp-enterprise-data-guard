#include "test_framework.hpp"

#include <iostream>

int main() {
    const auto& tests = dataguard::test::registry();
    for (const auto& tc : tests) {
        tc.fn();
    }
    int failures = dataguard::test::failureCount();
    std::cout << "Ran " << tests.size() << " test case(s), " << failures << " failure(s).\n";
    return failures == 0 ? 0 : 1;
}

