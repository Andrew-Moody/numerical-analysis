cmake_minimum_required(VERSION 3.13)

# Need to detect if X11 or Wayland is already installed but using X11 for now
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "Build for Wayland" FORCE)

# Disabling docs removes the need to install Doxygen
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation" FORCE)

# Load the submodule contents using the provided CMakeLists.txt
add_subdirectory(glfw)

# Override properties as needed
set_target_properties(glfw PROPERTIES FOLDER "Dependencies/GLFW3")
set_target_properties(uninstall PROPERTIES FOLDER "Dependencies/GLFW3")
set_target_properties(update_mappings PROPERTIES FOLDER "Dependencies/GLFW3")