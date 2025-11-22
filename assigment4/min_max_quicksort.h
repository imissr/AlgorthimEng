#ifndef MIN_MAX_QUICKSORT_H
#define MIN_MAX_QUICKSORT_H

#include <cstdint>

// Parallel quicksort (your implementation)
void min_max_quicksort(int64_t *arr, int64_t n, int num_threads = -1);

// Optional correctness test helper
bool verify_qs_correctness(int64_t size);

#endif // MIN_MAX_QUICKSORT_H
