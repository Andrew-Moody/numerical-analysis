cmake_minimum_required(VERSION 3.13)

project(numerical_analysis VERSION 1.0 LANGUAGES C CXX)

# Allow organization of targets into "folders" for Visual Studio
set_property(GLOBAL PROPERTY USE_FOLDERS ON)


# Adds the main target
add_subdirectory(numerical_analysis)

# Dependencies that must be installed
find_package(OpenGL REQUIRED)
find_package(OpenMP REQUIRED)

# Dependencies that are built alongside the main target
add_subdirectory(dependencies/glad)
add_subdirectory(dependencies/glfw)
add_subdirectory(dependencies/assimp)

# Linking Dependencies
target_link_libraries(numerical_analysis
    PRIVATE
        glad
        glfw
        assimp
        OpenMP::OpenMP_C
)

# Another Visual Studio convenience. Sets the target that is run with the "play" button
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT numerical_analysis)