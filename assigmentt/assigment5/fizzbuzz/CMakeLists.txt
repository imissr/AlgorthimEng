# Fizzbuzz static library

add_library(fizzbuzz STATIC
        src/fizzbuzz.cpp
)

target_include_directories(fizzbuzz
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
