add_executable(project_tests
        ops/test_otsu.cpp
        ops/test_sauvola.cpp
        ops/test_median.cpp
        io/test_ppm_io.cpp
)

target_link_libraries(project_tests PRIVATE
        project_lib
        Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(project_tests)
