cmake_minimum_required(VERSION 3.13)

target_sources(${TEST_TARGET_NAME}
    PRIVATE
        src/main.c
)

add_subdirectory(src/core)
add_subdirectory(src/graphics)
add_subdirectory(src/structural)
add_subdirectory(src/fluid)
