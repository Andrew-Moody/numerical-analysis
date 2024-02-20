cmake_minimum_required(VERSION 3.13)

target_sources(fluid_analysis
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
        matrix.h
)

target_include_directories(fluid_analysis
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}   
)
