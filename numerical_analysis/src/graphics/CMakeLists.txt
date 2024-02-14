cmake_minimum_required(VERSION 3.13)

target_sources(fluid_analysis
    PUBLIC
        graphics.h
        model.h
    PRIVATE
        graphics.c
)

target_include_directories(fluid_analysis
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}   
)
