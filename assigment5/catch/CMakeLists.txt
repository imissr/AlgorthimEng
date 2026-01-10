# Catch2 tests for fizzbuzz

enable_testing()

add_executable(catch_tests_fizzbuzz
        catch_main.cpp
        catch_tests_fizzbuzz.cpp
)

target_include_directories(catch_tests_fizzbuzz
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(catch_tests_fizzbuzz
        PRIVATE
        fizzbuzz
)

add_test(NAME catch_tests_fizzbuzz COMMAND catch_tests_fizzbuzz)
