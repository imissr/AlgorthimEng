#include "benchmark.h"
#include "min_max_quicksort.h"

#include <parallel/algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
#include <omp.h>
#include <thread>

#include "assigment4/benchmark.h"
#include "assigment4/min_max_quicksort.h"

using Clock = std::chrono::high_resolution_clock;

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



void benchmark::run() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);



    const std::vector<int64_t> test_sizes = {0, 1, 23, 133, 1777, 57462, 786453};
    for (auto n : test_sizes) {
        if (!verify_qs_correctness(n)) {
            std::cerr << "min_max_quicksort is incorrect for size " << n << "!\n";
            return;
        }
    }

    const std::vector<std::size_t> sizes = {
        100000,
        300000,
        1000000,
        3000000,
        10000000,  // >= 1e7
        30000000

    };

    const int max_threads = omp_get_max_threads();

    std::cout << "N,threads,time_std,time_minmax,time_gnu\n";

    for (std::size_t N : sizes) {
        std::vector<int64_t> base(N);
        fill_random(base);


        std::vector<int64_t> arr_std = base;
        double t_std = time_it([&] {
            __gnu_sequential::sort(arr_std.begin(), arr_std.end());
        });

        std::vector<int64_t> reference = arr_std;

        for (int threads = 1; threads <= max_threads; ++threads) {

            std::vector<int64_t> arr_mm = base;
            double t_mm = time_it([&] {
                min_max_quicksort(arr_mm.data(), static_cast<int64_t>(arr_mm.size()), threads);
            });

            if (arr_mm != reference) {
                std::cerr << "ERROR: min_max_quicksort wrong result for N="
                          << N << " threads=" << threads << "\n";
            }


            std::vector<int64_t> arr_gnu = base;
            omp_set_num_threads(threads);
            double t_gnu = time_it([&] {
                __gnu_parallel::sort(arr_gnu.begin(), arr_gnu.end());
            });

            if (arr_gnu != reference) {
                std::cerr << "ERROR: __gnu_parallel::sort wrong result for N="
                          << N << " threads=" << threads << "\n";
            }

            std::cout << N << "," << threads << ","
                      << t_std << "," << t_mm << "," << t_gnu << "\n";
        }
    }
}
