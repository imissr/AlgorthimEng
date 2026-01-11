add_executable(AlgEng
        io/token_reader.cpp
        io/token_reader.h
        io/ppm_reader.cpp
        io/ppm_reader.h
        src/main/main.cpp
)

target_include_directories(AlgEng
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(AlgEng PRIVATE algeng_core)
