cmake_minimum_required(VERSION 3.26)
project(AlgEng)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(AlgEng
        main.cpp
        assigment1/MonteCarlo.cpp
        assigment1/MonteCarlo.h
        assigment2/MonteCarloFor.cpp
        assigment2/MonteCarloFor.h
        assigment3/pi_monte_carlo_with_LCG.cpp
        assigment3/pi_monte_carlo_with_LCG.h
        assigment4/benchmark.cpp
        assigment4/benchmark.h
        assigment4/min_max_quicksort.cpp
        assigment4/min_max_quicksort.h
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

# GCC parallel mode (falls du __gnu_parallel::sort verwendest)
target_compile_definitions(AlgEng PRIVATE _GLIBCXX_PARALLEL)

# Python-Interpreter finden
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Custom Target: "plots"
add_custom_target(plots
        COMMAND AlgEng > results.csv
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/plot_speedup.py
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS AlgEng
        COMMENT "Running benchmark (AlgEng) and generating speedup plots"
)
