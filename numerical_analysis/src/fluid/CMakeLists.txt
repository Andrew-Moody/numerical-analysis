cmake_minimum_required(VERSION 3.13)

target_sources(${MAIN_TARGET_NAME}
    PRIVATE
        fluid.h
        fluid.c
)

target_include_directories(${MAIN_TARGET_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)