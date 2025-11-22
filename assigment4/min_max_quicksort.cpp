#include "min_max_quicksort.h"

#include <iostream>
#include <vector>
#include <limits>
#include <omp.h>
#include <parallel/algorithm>
#include <cstdint>

inline int64_t average(int64_t a, int64_t b) {
    return (a & b) + ((a ^ b) >> 1);
}


inline int64_t
partition(int64_t *arr, int64_t left, int64_t right, int64_t pivot,
          int64_t &smallest, int64_t &biggest) {
    int64_t *left_ptr = &arr[left];
    int64_t *right_ptr = &arr[right];
    while (left_ptr < right_ptr) {
        smallest = (*left_ptr < smallest) ? *left_ptr : smallest;
        biggest = (*left_ptr > biggest) ? *left_ptr : biggest;
        if (*left_ptr > pivot) {
            --right_ptr;
            std::swap(*left_ptr, *right_ptr);
        } else {
            ++left_ptr;
        }
    }
    return left_ptr - arr;
}

inline void insertion_sort(int64_t *arr, int64_t left, int64_t right) {
    for (int64_t i = left + 1; i <= right; i++) {
        int64_t key = arr[i];
        int64_t j = i - 1;
        while (j >= left && arr[j] > key) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

// the core recursive quicksort function
void qs_core(int64_t *arr, int64_t left, int64_t right, const int64_t pivot) {
    if (right - left < 32) {
        insertion_sort(arr, left, right);
        return;
    }

    int64_t smallest = std::numeric_limits<int64_t>::max();
    int64_t biggest = std::numeric_limits<int64_t>::min();
    int64_t bound = partition(arr, left, right + 1, pivot, smallest, biggest);

    if (smallest == biggest)
        return;

#pragma omp task final(bound - left < 10000)
    qs_core(arr, left, bound - 1, average(smallest, pivot));
    qs_core(arr, bound, right, average(pivot, biggest));
}

void min_max_quicksort(int64_t *arr, int64_t n, int num_threads) {
    if (n <= 1) return;
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }

#pragma omp parallel num_threads(num_threads)
#pragma omp single nowait
    qs_core(arr, 0, n - 1, arr[average(0, n - 1)]);
}

class Xoroshiro128Plus {
    uint64_t state[2]{};

    static inline uint64_t rotl(const uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
    }

public:
    explicit Xoroshiro128Plus(uint64_t seed = 0) {
        state[0] = (12345678901234567ULL + seed) |
                   0b1001000010000001000100101000000110010010100000011001001010000001ULL;
        state[1] = (98765432109876543ULL + seed) |
                   0b0100000011001100100000011001001010000000100100101000000110010010ULL;
        for (int i = 0; i < 10; i++) { operator()(); }
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

bool verify_qs_correctness(int64_t size) {
    Xoroshiro128Plus generator(size);
    std::vector<int64_t> data(size);
    for (int64_t i = 0; i < size; ++i) {
        data[i] = static_cast<int64_t>(generator());
    }
    std::vector<int64_t> data_copy = data;
    min_max_quicksort(data.data(), size);
    std::sort(data_copy.begin(), data_copy.end());
    return data == data_copy;
}
