# Quantize library (libimagequant)

file(GLOB QUANTIZE_SOURCES "*.c")

add_library(quantize STATIC
        ${QUANTIZE_SOURCES}
)

target_include_directories(quantize
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Explicitly set C as the language for this target
set_target_properties(quantize PROPERTIES LINKER_LANGUAGE C)

# Quantize library uses math functions
target_link_libraries(quantize
        PUBLIC
        m
)
