#include "pi_monte_carlo_with_LCG.h"
unsigned long long fib(unsigned n) {
    if (n <= 2) return 1ULL;
    return fib(n - 1) + fib(n - 2);
}

int main() {


    pi_monte_carlo_with_LCG test3;
    test3.pi_monte_carlo_LCG();

    printf("Hallo,world: ");
#pragma omp parallel
    printf(" %d", omp_get_thread_num ());
    printf("\n");



    int n =45;
#pragma omp parallel
    {
        int t = omp_get_thread_num();
        int k = n + t;

        if (k > 93) {
            printf("%d: fib(%d) overflows 64-bit\n", t, k);
        } else {
            printf("%d: %llu\n", t, fib(k));
        }
    }
}
