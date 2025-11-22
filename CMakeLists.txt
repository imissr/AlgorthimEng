cmake_minimum_required(VERSION 3.26)
project(AlgEng)

set(CMAKE_CXX_STANDARD 20)

add_executable(AlgEng
        main.cpp
        assigment1/MonteCarlo.cpp
        assigment1/MonteCarlo.h
        assigment2/MonteCarloFor.cpp
        assigment2/MonteCarloFor.h
        assigment3/pi_monte_carlo_with_LCG.cpp
        assigment3/pi_monte_carlo_with_LCG.h
)
add_executable(Test1
        assigment3/Test1.cpp
)
add_executable(Test2
        assigment3/Test2.cpp
)

find_package(OpenMP REQUIRED)
target_link_libraries(AlgEng PRIVATE OpenMP::OpenMP_CXX)
target_link_libraries(Test1 PRIVATE OpenMP::OpenMP_CXX)
target_link_libraries(Test2 PRIVATE OpenMP::OpenMP_CXX)
