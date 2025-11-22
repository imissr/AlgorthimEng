#include <iostream>
#include "assigment1/MonteCarlo.h"
#include "assigment2/MonteCarloFor.h"
#include "assigment3/pi_monte_carlo_with_LCG.h"

// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {
    MonteCarlo test;
    //test.monteCarlo();
    MonteCarloFor test2;
    //test2.monteCarloFor();
    pi_monte_carlo_with_LCG test3;
    test3.pi_monte_carlo_LCG();


    return 0;

}