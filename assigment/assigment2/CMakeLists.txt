cmake_minimum_required(VERSION 3.20)
project(Assignment2)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /Oi /Ot /GL /fp:fast")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi")
    add_compile_options(/W4)
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -mtune=native -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "-Og -g")
    add_compile_options(-Wall -Wextra -pedantic -fopenmp)
endif()
find_package(OpenMP REQUIRED)

# Assignment 2 executable
add_executable(main_assigment2 main_assigment2.cpp)
if(NOT MSVC)
    target_link_libraries(main_assigment2 PRIVATE -fopenmp)
else()
    target_link_libraries(main_assigment2 PRIVATE OpenMP::OpenMP_CXX)
endif()