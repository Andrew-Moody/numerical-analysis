cmake_minimum_required(VERSION 3.13)

target_sources(${MAIN_TARGET_NAME}
    PRIVATE
        filepath.h
        filepath.c
        transform.h
        vector.h
        matrix.h
        linearsolve.h
        linearsolve.c
        mpitest.h
        mpiutility.h
        mpiutility.c
        linsolvempi.h
        linsolvempi.c
        api.h
        api.c
)

target_include_directories(${MAIN_TARGET_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)