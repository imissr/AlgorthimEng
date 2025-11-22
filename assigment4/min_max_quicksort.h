#ifndef MIN_MAX_QUICKSORT_H
#define MIN_MAX_QUICKSORT_H

#include <cstdint>

void min_max_quicksort(int64_t *arr, int64_t n, int num_threads = -1);

bool verify_qs_correctness(int64_t size);

#endif // MIN_MAX_QUICKSORT_H
