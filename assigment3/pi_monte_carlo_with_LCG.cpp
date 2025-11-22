//
// Created by khale on 22.11.2025.
//

#include "pi_monte_carlo_with_LCG.h"

double rnd(unsigned int &seed) {
    seed = (1140671485u * seed + 12820163u) % (1u << 24);
    return static_cast<double>(seed) / static_cast<double>(1u << 24);
}

void pi_monte_carlo_with_LCG::pi_monte_carlo_LCG() {
    int n = 100000000000000;
    omp_set_num_threads(10);
    int counter = 0;
    double start_time = omp_get_wtime();

#pragma omp parallel reduction(+:counter)
    {

        unsigned int seed = omp_get_thread_num();

#pragma omp for
        for (int i = 0; i < n; ++i) {
            double x = rnd(seed);
            double y = rnd(seed);
            if (x * x + y * y <= 1.0) {
                ++counter;
            }
        }
    }

    double run_time = omp_get_wtime() - start_time;
    double pi = 4.0 * (double(counter) / n);

    std::cout << "pi: "       << pi       << '\n';
    std::cout << "run_time: " << run_time << " s\n";
    std::cout << "n: "        << n        << '\n';

}



