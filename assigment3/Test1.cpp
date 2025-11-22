//
// Created by khale on 19.11.2025.
//

#include <stdio.h>
#include <omp.h>

int main() {
    printf("Hallo,world: ");
#pragma omp parallel
    printf(" %d", omp_get_thread_num ());
printf("\n");
    return 0;
}