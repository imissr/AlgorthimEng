# LodePNG library

add_library(lodepng STATIC
        lodepng.cpp
        lodepng.h
)

target_include_directories(lodepng
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
