//
// Created by khale on 22.11.2025.
//

#ifndef ALGENG_BENCHMARK_H
#define ALGENG_BENCHMARK_H

#include <thread>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __linux__
#include <unistd.h>  // sysconf for RAM on Linux
#endif
class benchmark {
public:


    // Führt alle Messungen aus und schreibt CSV auf stdout
    static void run();
};

#endif // ALGENG_BENCHMARK_H
