cmake_minimum_required(VERSION 3.26 FATAL_ERROR)
project(AlgEng LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenMP REQUIRED)
message(STATUS "OpenMP CXX flags: ${OpenMP_CXX_FLAGS}")
message(STATUS "OpenMP libraries: ${OpenMP_CXX_LIBRARIES}")

include(FetchContent)

# Configure Catch2 to not build its own tests and examples
set(CATCH_INSTALL_DOCS OFF CACHE BOOL "")
set(CATCH_INSTALL_EXTRAS OFF CACHE BOOL "")
set(CATCH_BUILD_TESTING OFF CACHE BOOL "")
set(CATCH_BUILD_EXAMPLES OFF CACHE BOOL "")
set(CATCH_BUILD_EXTRA_TESTS OFF CACHE BOOL "")

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG devel
)
FetchContent_MakeAvailable(Catch2)

enable_testing()
include(CTest)

# Add assignment subdirectories
add_subdirectory(assigment/assigment1)
add_subdirectory(assigment/assigment2)
add_subdirectory(assigment/assigment3)
add_subdirectory(assigment/assigment4)
add_subdirectory(assigment/assigment5)

# Add main project
add_subdirectory(project)
