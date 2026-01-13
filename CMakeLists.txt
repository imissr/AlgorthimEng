cmake_minimum_required(VERSION 3.26 FATAL_ERROR)
project(AlgEng LANGUAGES CXX C)

# ---- C++ standard ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- OpenMP ----
find_package(OpenMP REQUIRED)

# ---- Core library (NO mains here) ----
add_library(algeng_core STATIC
        assigment1/MonteCarlo.cpp
        assigment2/MonteCarloFor.cpp
        assigment3/pi_monte_carlo_with_LCG.cpp
        assigment4/benchmark.cpp
        assigment4/min_max_quicksort.cpp

)

# Headers reachable via project root (e.g. #include "assigment1/MonteCarlo.h")
target_include_directories(algeng_core
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link OpenMP once to the core lib
target_link_libraries(algeng_core
        PUBLIC
        OpenMP::OpenMP_CXX
)

# Enable GCC parallel mode only when using g++
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_definitions(algeng_core PUBLIC _GLIBCXX_PARALLEL)
endif ()

# ---- Executables (each has exactly one main) ----
add_executable(main_assigment1
        assigment1/main_assigment1.cpp
)
target_link_libraries(main_assigment1 PRIVATE algeng_core)


add_executable(main_assigment2 assigment2/main_assigment2.cpp)
target_link_libraries(main_assigment2 PRIVATE algeng_core)


add_executable(main_assigment3
        assigment3/main_assigment3.cpp
)
target_link_libraries(main_assigment3 PRIVATE algeng_core)

add_executable(main_assigment4 assigment4/main_assigment4.cpp)
target_link_libraries(main_assigment4 PRIVATE algeng_core)



# ---- Subdirectories ----
add_subdirectory(assigment5)
add_subdirectory(project)
