//
// Created by khale on 19.11.2025.
//

#include "assigment2/MonteCarloFor.h"

using namespace std;


void MonteCarloFor::monteCarloFor() {

    int n = 10000000; // Anzahl der Zufallspunkte
    int counter = 0; // Punkte im Viertelkreis

    double start_time = omp_get_wtime();
    omp_set_num_threads(5);

#pragma omp parallel
    {
        // Jeder Thread bekommt seinen eigenen Zufallszahlengenerator
        unsigned int seed = 0 + omp_get_thread_num();
        default_random_engine re(seed);
        uniform_real_distribution<double> zero_to_one(0.0, 1.0);

        int local_counter = 0; // lokaler Zähler pro Thread

#pragma omp for
        for (int i = 0; i < n; ++i) {
            double x = zero_to_one(re);
            double y = zero_to_one(re);

            if (x * x + y * y <= 1.0) {
                ++local_counter;
            }
        }


#pragma omp atomic
        counter += local_counter;
    }

    double run_time = omp_get_wtime() - start_time;
    double pi = 4.0 * static_cast<double>(counter) / static_cast<double>(n);

    cout << "pi: " << setprecision(17) << pi << "\n";
    cout << "run_time: " << setprecision(6) << run_time << " s\n";
    cout << "n: " << n << "\n";

}