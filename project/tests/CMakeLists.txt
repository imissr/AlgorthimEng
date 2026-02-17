add_executable(project_tests
        ops/test_otsu.cpp
        ops/test_sauvola.cpp
        ops/test_median.cpp
        ops/test_nick.cpp
        ops/test_threshold_proposed.cpp
        ops/test_threshold_su.cpp
        ops/test_background_estimate.cpp
        ops/test_background_remove.cpp
        ops/test_border_cleanup.cpp
        ops/test_contrast_stretch.cpp
        ops/test_morphology.cpp
        io/test_ppm_io.cpp
)

target_link_libraries(project_tests PRIVATE
        project_lib
        Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(project_tests)
