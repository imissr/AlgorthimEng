#include "pi_monte_carlo_with_LCG.h"
long fib(int n) {
    if (n == 1)
        return 1;
    if (n == 2)
        return 1;
    return fib(n-1)+ fib(n-2);

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
        printf("%d: %ld\n "  , t ,fib(n + t));
    }
    return 0;

}
