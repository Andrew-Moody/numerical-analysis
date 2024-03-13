cmake_minimum_required(VERSION 3.13)

project(numerical_analysis VERSION 1.0 LANGUAGES C CXX)

# Allow organization of targets into "folders" for Visual Studio
set_property(GLOBAL PROPERTY USE_FOLDERS ON)



# Add the main target
set(MAIN_TARGET_NAME "numerical_analysis")
add_executable(${MAIN_TARGET_NAME} "")
# Since the target is added here instead of in the numerical_analysis subdirectory
# the output directory must be set to keep the executable under numerical_analysis
set_target_properties(${MAIN_TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY numerical_analysis)



# Dependencies that must be installed
find_package(OpenGL REQUIRED)
find_package(OpenMP REQUIRED)

# Dependencies that are built alongside the main target
add_subdirectory(dependencies/glad)
add_subdirectory(dependencies/glfw)
add_subdirectory(dependencies/assimp)

# Linking Dependencies
target_link_libraries(${MAIN_TARGET_NAME}
    PRIVATE
        glad
        glfw
        assimp
        OpenMP::OpenMP_C
)


# Add sources for the main target
add_subdirectory(numerical_analysis)

# Optional Dependencies
# Use the following command from the build directory when running cmake to enable MPI
# cmake ../ -DENABLE_MPI=true
# to disable MPI again use
# cmake ../ -DENABLE_MPI=false
if(NOT DEFINED ENABLE_MPI)
    set(ENABLE_MPI "false")
endif()

add_compile_definitions(ENABLE_MPI=0)
if(${ENABLE_MPI})
    find_package(MPI)
    if(${MPI_FOUND})
        add_compile_definitions(ENABLE_MPI=1)
        target_link_libraries(${MAIN_TARGET_NAME} PRIVATE MPI::MPI_C)
    else()
        message(WARNING "   ENABLE_MPI is set to true but cmake failed to find MPI. MPI functionality will be disabled")
    endif()
endif()


# Another Visual Studio convenience. Sets the target that is run with the "play" button
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT numerical_analysis)