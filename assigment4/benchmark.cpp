#include "benchmark.h"
#include "min_max_quicksort.h"

#include <parallel/algorithm>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
#include <omp.h>

#include "assigment4/benchmark.h"
#include "assigment4/min_max_quicksort.h"

template <class F>
double time_it(F&& f) {
    double start = omp_get_wtime();
    f();
    double end = omp_get_wtime();
    return end - start;
}

class Xoroshiro128Plus {
    uint64_t state[2]{};

    static inline uint64_t rotl(const uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
    }

public:
    explicit Xoroshiro128Plus(uint64_t seed = 0) {
        state[0] = (12345678901234567 + seed) | 0b1001000010000001000100101000000110010010100000011001001010000001ULL;
        state[1] = (98765432109876543 + seed) | 0b0100000011001100100000011001001010000000100100101000000110010010ULL;
        for(int i = 0; i < 10; i++){operator()();}
    }

    uint64_t operator()() {
        const uint64_t s0 = state[0];
        uint64_t s1 = state[1];
        const uint64_t result = s0 + s1;

        s1 ^= s0;
        state[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16);
        state[1] = rotl(s1, 37);
        return result;
    }
};

// Fill a vector with reproducible random data (same as before, but simpler RNG)
static void fill_random(std::vector<int64_t>& v) {
    Xoroshiro128Plus generator(500000);
    for (int64_t i = 0; i < v.size(); ++i) {
        v[i] = static_cast<int64_t>(generator());
    }
}



void benchmark::run() {
    // std::ios::sync_with_stdio(false);
    // std::cin.tie(nullptr);



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
        double t_std = time_it([&] {
            std::sort(base.begin(), base.end());
        });


        for (int threads = 1; threads <= max_threads; ++threads) {

            std::vector<int64_t> arr_mm(N);
            fill_random(arr_mm);
            double t_mm = time_it([&] {
                min_max_quicksort(arr_mm.data(), static_cast<int64_t>(arr_mm.size()), threads);
            });


            std::vector<int64_t> arr_gnu(N);
            fill_random(arr_gnu);
            double t_gnu = time_it([&] {
                __gnu_parallel::sort(arr_gnu.begin(), arr_gnu.end(), __gnu_parallel::parallel_tag(threads));
            });

            std::cout << N << "," << threads << ","
                      << t_std << "," << t_mm << "," << t_gnu << "\n";
        }
    }
}
