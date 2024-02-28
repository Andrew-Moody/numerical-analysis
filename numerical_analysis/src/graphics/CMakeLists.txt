cmake_minimum_required(VERSION 3.13)

target_sources(numerical_analysis
    PRIVATE
        graphics.c
        mesh.c
        model.c
        import.c
        shader.c
        camera.c
        graphics.h
        mesh.h
        model.h
        import.h
        shader.h
        camera.h
)

target_include_directories(numerical_analysis
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}   
)
