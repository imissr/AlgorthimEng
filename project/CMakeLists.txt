# 1) Put all "real code" into a library (NO main.cpp)
add_library(project_lib STATIC
        src/io/token_reader.cpp
        src/io/ppm_reader.cpp
        src/io/ppm_writer.cpp
        src/image/image_gray.cpp
        src/image/colorspace.cpp
        src/ops/denoise_median.cpp
        src/ops/background_estimate.cpp
        src/ops/background_remove.cpp
        src/ops/contrast_stretch.cpp
        src/ops/threshold_otsu.cpp
        src/ops/threshold_sauvola.cpp
        src/ops/threshold_nick.cpp
        src/ops/morphology.cpp
        src/ops/border_cleanup.cpp
        src/util/timing.cpp
        src/util/logging.cpp
        src/util/args_parser.cpp
        src/ops/threshold_su.cpp
        src/ops/threshold_su.h
)

target_include_directories(project_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(project_lib PUBLIC
        algeng_core
)

# 2) Executable only contains main.cpp and links to the library
add_executable(project_executable
        src/main/main.cpp
)

target_link_libraries(project_executable PRIVATE project_lib)

# 3) Tests
add_subdirectory(tests)
