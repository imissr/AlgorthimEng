#ifndef ALGENG_UTIL_TIMING_H
#define ALGENG_UTIL_TIMING_H

#include <chrono>
#include <string>

class Timer {
public:
    explicit Timer(std::string name);
    ~Timer(); // prints elapsed time on destruction

private:
    std::string name_;
    std::chrono::steady_clock::time_point start_;
};

#endif
