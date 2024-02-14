cmake_minimum_required(VERSION 3.13)

# Enable/Disable optional features
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build libraries as shared" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Build test suite" FORCE)

set(ASSIMP_NO_EXPORTER ON CACHE BOOL "Disable Assimp's export functionality" FORCE)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "Build Assimp with all exporters enabled" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "Build Assimp with all importers enabled" FORCE)

set(ASSIMP_BUILD_STL_IMPORTER ON CACHE BOOL "Build Assimp with all STL importer enabled" FORCE)

# Use CMakeLists.txt provided with Assimp
add_subdirectory(assimp)

# Organize Targets into folders for tidy Visual Studio Solution
set_target_properties(assimp PROPERTIES FOLDER "Dependencies/Assimp")
#set_target_properties(UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER "Dependencies/Assimp")
#set_target_properties(zlibstatic PROPERTIES FOLDER "Dependencies/Assimp")