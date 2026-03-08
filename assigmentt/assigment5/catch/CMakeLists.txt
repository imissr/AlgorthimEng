# Catch2 tests for fizzbuzz (using global FetchContent Catch2)

add_executable(catch_tests_fizzbuzz
        catch_tests_fizzbuzz.cpp
)

target_link_libraries(catch_tests_fizzbuzz
        PRIVATE
        fizzbuzz
        Catch2::Catch2WithMain
)

# If your test includes headers from fizzbuzz/include, linking fizzbuzz is enough,
# because fizzbuzz exports its include dir PUBLIC. So you usually don't need:
# target_include_directories(catch_tests_fizzbuzz ...)

include(CTest)
include(Catch)
catch_discover_tests(catch_tests_fizzbuzz)
