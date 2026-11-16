#include "core/validators.hpp"
#include "test_framework.hpp"

using namespace dataguard::validators;

TEST_CASE("validators entropy") {
    CHECK_NEAR(shannonEntropy(""), 0.0, 1e-9);
    CHECK_NEAR(shannonEntropy("aaaaaa"), 0.0, 1e-9);
    CHECK(shannonEntropy("aB3$xY9!kL2@mN7#") > 3.5);
}

TEST_CASE("validators luhn") {
    CHECK(isValidLuhn("4532-0151-1283-0366"));
    CHECK(isValidLuhn("4532 0151 1283 0366"));
    CHECK(!isValidLuhn("4532-0151-1283-0367"));
    CHECK(!isValidLuhn("0000000000000000"));
    CHECK(!isValidLuhn("123456"));
    CHECK(!isValidLuhn("4532abcd12830366"));
}

TEST_CASE("validators iban") {
    CHECK(isValidIban("DE89 3704 0044 0532 0130 00"));
    CHECK(isValidIban("GB82WEST12345698765432"));
    CHECK(!isValidIban("DE89 3704 0044 0532 0130 01"));
    CHECK(!isValidIban("DE89-123"));
    CHECK(!isValidIban("1289370400440532013000"));
    CHECK(!isValidIban("DE8937040044053201300$"));
}

TEST_CASE("validators ssn") {
    CHECK(isValidSsn("123-45-6789"));
    CHECK(!isValidSsn("123456789"));
    CHECK(!isValidSsn("123/45/6789"));
    CHECK(!isValidSsn("12a-45-6789"));
    CHECK(!isValidSsn("000-45-6789"));
    CHECK(!isValidSsn("666-45-6789"));
    CHECK(!isValidSsn("900-45-6789"));
    CHECK(!isValidSsn("123-00-6789"));
    CHECK(!isValidSsn("123-45-0000"));
}

TEST_CASE("validators maskSecret") {
    CHECK_EQ(maskSecret("short"), "****");
    CHECK_EQ(maskSecret("AKIA1234567890ABCDEF", 4, 4), "AKIA****CDEF");
}

