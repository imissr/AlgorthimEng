add_executable(AlgEng
        src/io/token_reader.cpp
        src/io/token_reader.h
        src/io/ppm_reader.cpp
        src/io/ppm_reader.h
        src/main/main.cpp
        src/io/ppm_writer.cpp
        src/io/ppm_writer.h
        src/image/image_gray.cpp
        src/image/image_gray.h
        src/image/colorspace.cpp
        src/image/colorspace.h
        src/ops/denoise_median.cpp
        src/ops/denoise_median.h
        src/ops/background_estimate.cpp
        src/ops/background_estimate.h
        src/ops/background_remove.cpp
        src/ops/background_remove.h
        src/ops/contrast_stretch.cpp
        src/ops/contrast_stretch.h
        src/util/clamp.h
        src/util/timing.h
        src/util/timing.cpp
        src/util/logging.cpp
        src/util/logging.h
        src/util/args_parser.h
        src/util/args_parser.cpp
)

target_include_directories(AlgEng
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(AlgEng PRIVATE algeng_core)
