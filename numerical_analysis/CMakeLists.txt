cmake_minimum_required(VERSION 3.13)

find_package(OpenMP REQUIRED)

add_executable(fluid_analysis "")

target_sources(fluid_analysis
    PRIVATE
        src/main.c
        src/fluid.h
        src/fluid.c
)

add_subdirectory(src/graphics)

target_link_libraries(fluid_analysis
    PRIVATE 
        OpenMP::OpenMP_C
)
