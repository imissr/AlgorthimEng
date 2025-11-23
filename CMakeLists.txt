cmake_minimum_required(VERSION 3.26 FATAL_ERROR)
project(AlgEng LANGUAGES CXX)

# ---- C++ standard, like in the slides ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # no compiler-specific extensions

# ---- OpenMP (imported target) ----
find_package(OpenMP REQUIRED)

# ---- Core library with all assignment code ----
add_library(algeng_core STATIC
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

# All headers are reachable via the project root (e.g. #include "assigment1/MonteCarlo.h")
target_include_directories(algeng_core
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link OpenMP only once to the core lib, like the picomp example
target_link_libraries(algeng_core
        PUBLIC
        OpenMP::OpenMP_CXX
)

# Enable GCC parallel mode only when using g++
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_definitions(algeng_core PUBLIC _GLIBCXX_PARALLEL)
endif ()

# ---- Executables ----

add_executable(AlgEng
        main.cpp
)
target_link_libraries(AlgEng PRIVATE algeng_core)

add_executable(Test1
        assigment3/Test1.cpp
)
target_link_libraries(Test1 PRIVATE algeng_core)

add_executable(Test2
        assigment3/Test2.cpp
)
target_link_libraries(Test2 PRIVATE algeng_core)

