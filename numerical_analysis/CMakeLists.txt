cmake_minimum_required(VERSION 3.22)

project(numerical_analysis VERSION 1.0 LANGUAGES C)

find_package(OpenMP REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.c
    src/fluid.c)

target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_C)
