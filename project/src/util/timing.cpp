#include "timing.h"
#include <iostream>

Timer::Timer(std::string name)
    : name_(std::move(name)), start_(std::chrono::steady_clock::now()) {}

Timer::~Timer() {
    using namespace std::chrono;
    auto end = steady_clock::now();
    auto ms = duration_cast<milliseconds>(end - start_).count();
    std::cout << "[time] " << name_ << ": " << ms << " ms\n";
}
