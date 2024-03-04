cmake_minimum_required(VERSION 3.13)

target_sources(numerical_analysis
    PRIVATE
        transform.h
        vector.h
        matrix.h
        linearsolve.h
        linearsolve.c
)

target_include_directories(numerical_analysis
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)