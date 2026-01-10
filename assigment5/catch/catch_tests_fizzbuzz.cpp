#include "catch.hpp"
#include "fizzbuzz.h"

#include <string>
#include <vector>

TEST_CASE("fizzbuzz_value handles fizz, buzz, and numbers") {
    REQUIRE(fizzbuzz_value(1) == "1");
    REQUIRE(fizzbuzz_value(3) == "fizz");
    REQUIRE(fizzbuzz_value(5) == "buzz");
    REQUIRE(fizzbuzz_value(15) == "fizzbuzz");
}

TEST_CASE("fizzbuzz returns the sequence from 1 to n") {
    const std::vector<std::string> expected{
            "1", "2", "fizz", "4", "buzz",
            "fizz", "7", "8", "fizz", "buzz",
            "11", "fizz", "13", "14", "fizzbuzz"
    };

    REQUIRE(fizzbuzz(15) == expected);
}

TEST_CASE("fizzbuzz returns empty for non-positive n") {
    REQUIRE(fizzbuzz(0).empty());
    REQUIRE(fizzbuzz(-4).empty());
}
