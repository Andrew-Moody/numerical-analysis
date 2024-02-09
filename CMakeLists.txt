cmake_minimum_required(VERSION 3.13)

project(numerical_analysis VERSION 1.0 LANGUAGES C)

find_package(OpenGL REQUIRED)

# Allow organization of targets into "folders" for Visual Studio
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(numerical_analysis)

add_subdirectory(dependencies/glad)

add_subdirectory(dependencies/glfw)

target_link_libraries(fluid_analysis
    PRIVATE
        glad
        glfw
)

# Another Visual Studio convenience. Sets the target that is run with the "play" button
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT fluid_analysis)