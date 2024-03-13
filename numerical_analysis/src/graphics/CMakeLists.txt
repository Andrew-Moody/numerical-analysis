cmake_minimum_required(VERSION 3.13)

target_sources(${MAIN_TARGET_NAME}
    PRIVATE
        graphics.c
        mesh.c
        model.c
        import.c
        shader.c
        camera.c
        input.c
        application.c
        graphics.h
        mesh.h
        model.h
        import.h
        shader.h
        camera.h
        input.h
        application.h
)

target_include_directories(${MAIN_TARGET_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}   
)
