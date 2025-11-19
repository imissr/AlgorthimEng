cmake_minimum_required(VERSION 3.26)
project(AlgEng)

set(CMAKE_CXX_STANDARD 20)

add_executable(AlgEng
        main.cpp
        assigment1/MonteCarlo.cpp
        assigment1/MonteCarlo.h
        assigment2/MonteCarloFor.cpp
        assigment2/MonteCarloFor.h
)

find_package(OpenMP REQUIRED)
target_link_libraries(AlgEng PRIVATE OpenMP::OpenMP_CXX)
