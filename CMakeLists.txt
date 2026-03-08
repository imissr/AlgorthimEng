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


# Compiler-specific flags
if(MSVC)
    # MSVC: /O2 = optimize, /Ob2 = inline, /DNDEBUG in release
    add_compile_options(/W4)

    set(CMAKE_C_FLAGS_RELEASE          "/O2 /Ob2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE        "/O2 /Ob2 /DNDEBUG")

    set(CMAKE_C_FLAGS_RELWITHDEBINFO   "/O2 /Ob2 /Zi /DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Ob2 /Zi /DNDEBUG")

else()
    # GCC/Clang
    add_compile_options(-Wall -Wextra -Wpedantic)

    # Release: strong optimization
    set(CMAKE_C_FLAGS_RELEASE          "-O3 -DNDEBUG -march=native -mtune=native -ffast-math -funroll-loops")
    set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG -march=native -mtune=native -ffast-math -funroll-loops")

    # RelWithDebInfo: optimized but debuggable
    set(CMAKE_C_FLAGS_RELWITHDEBINFO   "-O2 -g -DNDEBUG -march=native -mtune=native")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG -march=native -mtune=native")

    # Debug: no optimization, best debug experience
    set(CMAKE_C_FLAGS_DEBUG            "-O0 -g")
    set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g")
endif()

# Add assignment subdirectories
add_subdirectory(assigmentt/assigment1)
add_subdirectory(assigmentt/assigment2)
add_subdirectory(assigmentt/assigment3)
add_subdirectory(assigmentt/assigment4)
add_subdirectory(assigmentt/assigment5)

# Add main project
add_subdirectory(project)
