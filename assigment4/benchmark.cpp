#include "benchmark.h"
#include "min_max_quicksort.h"

#include <parallel/algorithm>   // __gnu_parallel::sort, __gnu_sequential::sort
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
#include <omp.h>
#include <thread>

using Clock = std::chrono::high_resolution_clock;

// Helper: time a function (seconds)
template <class F>
double time_it(F&& f) {
    auto start = Clock::now();
    f();
    auto end = Clock::now();
    std::chrono::duration<double> diff = end - start;
    return diff.count();
}

// Fill a vector with reproducible random data (same as before, but simpler RNG)
static void fill_random(std::vector<int64_t>& v) {
    std::mt19937_64 rng(42);
    std::uniform_int_distribution<int64_t> dist(
        std::numeric_limits<int64_t>::min(),
        std::numeric_limits<int64_t>::max()
    );
    for (auto &x : v) x = dist(rng);
}
static void print_environment_info() {
    // ----- OS name -----
    std::string os_name = "Unknown";
#if defined(_WIN32)
    os_name = "Windows";
#elif defined(__linux__)
    os_name = "Linux";
#elif defined(__APPLE__)
    os_name = "macOS";
#endif

    // ----- CPU name (best-effort) -----
    std::string cpu_name = "Unknown";

#if defined(__linux__)
    // Read from /proc/cpuinfo
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.rfind("model name", 0) == 0) {
            auto pos = line.find(':');
            if (pos != std::string::npos && pos + 2 < line.size()) {
                cpu_name = line.substr(pos + 2);
            }
            break;
        }
    }
#elif defined(_WIN32)
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);

    std::string arch = "Unknown arch";
    switch (sysinfo.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_AMD64: arch = "x86_64"; break;
    case PROCESSOR_ARCHITECTURE_INTEL: arch = "x86"; break;
    case PROCESSOR_ARCHITECTURE_ARM:   arch = "ARM"; break;
    case PROCESSOR_ARCHITECTURE_ARM64: arch = "ARM64"; break;
    default: break;
    }

    unsigned hw_threads = std::thread::hardware_concurrency();
    cpu_name = arch + " (" + std::to_string(hw_threads) + " logical cores)";
#endif

    // ----- RAM (best-effort) -----
    std::string ram_str = "Unknown";

#if defined(__linux__)
    long pages     = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    if (pages > 0 && page_size > 0) {
        double ram_gb = static_cast<double>(pages) * page_size
                        / (1024.0 * 1024.0 * 1024.0);
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << ram_gb << " GB";
        ram_str = oss.str();
    }
#elif defined(_WIN32)
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (GlobalMemoryStatusEx(&statex)) {
        double ram_gb = static_cast<double>(statex.ullTotalPhys)
                        / (1024.0 * 1024.0 * 1024.0);
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << ram_gb << " GB";
        ram_str = oss.str();
    }
#endif

    // ----- Compiler -----
    std::string compiler = "Unknown";
#if defined(__clang__)
    compiler = std::string("Clang ") +
               std::to_string(__clang_major__) + "." +
               std::to_string(__clang_minor__) + "." +
               std::to_string(__clang_patchlevel__);
#elif defined(__GNUC__)
    compiler = std::string("GCC ") +
               std::to_string(__GNUC__) + "." +
               std::to_string(__GNUC_MINOR__) + "." +
               std::to_string(__GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
    compiler = std::string("MSVC ") + std::to_string(_MSC_VER);
#endif

     hw_threads = std::thread::hardware_concurrency();

    // Print to stderr so stdout remains clean CSV
    std::cerr << "# Benchmark environment:\n"
              << "#   OS      : " << os_name << "\n"
              << "#   CPU     : " << cpu_name << "\n"
              << "#   RAM     : " << ram_str << "\n"
              << "#   Cores   : " << hw_threads << "\n"
              << "#   Compiler: " << compiler << "\n\n";
}


void benchmark::run() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    print_environment_info();  // <--- add this

    // Optional: correctness sanity check
    const std::vector<int64_t> test_sizes = {0, 1, 23, 133, 1777, 57462, 786453};
    for (auto n : test_sizes) {
        if (!verify_qs_correctness(n)) {
            std::cerr << "min_max_quicksort is incorrect for size " << n << "!\n";
            return;
        }
    }

    // Array sizes (last one >= 1e7 as required)
    const std::vector<std::size_t> sizes = {
        100000,
        300000,
        1000000,
        3000000,
        10000000,  // >= 1e7
        30000000

    };

    const int max_threads = omp_get_max_threads();

    // CSV header: same format as before
    std::cout << "N,threads,time_std,time_minmax,time_gnu\n";

    for (std::size_t N : sizes) {
        std::vector<int64_t> base(N);
        fill_random(base);

        // --- Baseline: std::sort (sequential) ---
        // Use __gnu_sequential::sort to avoid accidental parallelism
        std::vector<int64_t> arr_std = base;
        double t_std = time_it([&] {
            __gnu_sequential::sort(arr_std.begin(), arr_std.end());
        });

        std::vector<int64_t> reference = arr_std;

        for (int threads = 1; threads <= max_threads; ++threads) {
            // --- min_max_quicksort ---
            std::vector<int64_t> arr_mm = base;
            double t_mm = time_it([&] {
                min_max_quicksort(arr_mm.data(), static_cast<int64_t>(arr_mm.size()), threads);
            });

            if (arr_mm != reference) {
                std::cerr << "ERROR: min_max_quicksort wrong result for N="
                          << N << " threads=" << threads << "\n";
            }

            // --- __gnu_parallel::sort ---
            std::vector<int64_t> arr_gnu = base;
            omp_set_num_threads(threads); // control threads for parallel mode
            double t_gnu = time_it([&] {
                __gnu_parallel::sort(arr_gnu.begin(), arr_gnu.end());
            });

            if (arr_gnu != reference) {
                std::cerr << "ERROR: __gnu_parallel::sort wrong result for N="
                          << N << " threads=" << threads << "\n";
            }

            // CSV line
            std::cout << N << "," << threads << ","
                      << t_std << "," << t_mm << "," << t_gnu << "\n";
        }
    }
}
