cmake_minimum_required(VERSION 3.26 FATAL_ERROR)
project(AlgEng LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenMP REQUIRED)

include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.4
)
FetchContent_MakeAvailable(Catch2)

enable_testing()
include(CTest)

add_library(algeng_core STATIC
        assigment1/MonteCarlo.cpp
        assigment2/MonteCarloFor.cpp
        assigment3/pi_monte_carlo_with_LCG.cpp
        assigment4/benchmark.cpp
        assigment4/min_max_quicksort.cpp
)

target_include_directories(algeng_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(algeng_core PUBLIC OpenMP::OpenMP_CXX)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_definitions(algeng_core PUBLIC _GLIBCXX_PARALLEL)
endif()

function(add_assignment_exe exe_name source_file)
    add_executable(${exe_name} ${source_file})
    target_link_libraries(${exe_name} PRIVATE algeng_core)
endfunction()

add_assignment_exe(main_assigment1 assigment1/main_assigment1.cpp)
add_assignment_exe(main_assigment2 assigment2/main_assigment2.cpp)
add_assignment_exe(main_assigment3 assigment3/main_assigment3.cpp)
add_assignment_exe(main_assigment4 assigment4/main_assigment4.cpp)

add_subdirectory(assigment5)
add_subdirectory(project)
