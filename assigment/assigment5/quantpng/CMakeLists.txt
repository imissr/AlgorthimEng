# Quantpng executable - Image quantization with PNG support

# Add subdirectories for libraries
add_subdirectory(lodepng)
add_subdirectory(quantize)

# Create the quantpng executable
add_executable(quantpng
        main.cpp
)

# Link libraries
target_link_libraries(quantpng
        PRIVATE
        lodepng
        quantize
        OpenMP::OpenMP_CXX
)

# Include directories
target_include_directories(quantpng
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/lodepng
        ${CMAKE_CURRENT_SOURCE_DIR}/quantize
)

# Compile flags for optimization and OpenMP
target_compile_options(quantpng
        PRIVATE
        -O2
)
