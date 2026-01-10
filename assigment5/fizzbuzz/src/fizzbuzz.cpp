#include "fizzbuzz.h"

#include <string>
#include <vector>

std::string fizzbuzz_value(int i) {
    std::string result;
    if (i % 3 == 0) {
        result = "fizz";
    }
    if (i % 5 == 0) {
        result += "buzz";
    }
    if (result.empty()) {
        result = std::to_string(i);
    }
    return result;
}

std::vector<std::string> fizzbuzz(int n) {
    std::vector<std::string> out;
    if (n <= 0) {
        return out;
    }

    out.reserve(static_cast<size_t>(n));
    for (int i = 1; i <= n; ++i) {
        out.push_back(fizzbuzz_value(i));
    }
    return out;
}
