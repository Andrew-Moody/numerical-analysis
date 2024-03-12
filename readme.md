# Structural Frame Finite Element Analysis

![Car Frame](carframe.png)

FSAE car frame with deflection

### Dependencies and Build instructions
This project has currently only been tested on Ubuntu but I will be trying to test on other distributions and potentially adding a windows version as well.

#### TLDR
    sudo apt update  
    sudo apt install cmake  
    sudo apt install mesa-utils  
    git clone --recurse-submodules https://github.com/Andrew-Moody/numerical-analysis.git
    cd [repo location]/build
    cmake ../
    cmake --build [repo location]/build
    cd [repo location]/build/numerical_analysis
    ./numerical_analysis

Makes you almost miss good old windows installation wizards.

#### More Details
Ubuntu comes with gcc which should support OpenMP out of the box. You just need to have the -fopenmp compiler flag set which should be handled correctly with the provided CMake files.

You will need to have CMake installed to build. I tried to keep the minimum version reasonable but there are a few features that require 3.13 or above. If the version of ubuntu you have is recent it should come with a high enough version to install directly. It is recommended to check for package updates first.

    sudo apt update
    sudo apt install cmake

You will also need to have OpenGL support for graphics. If you use a desktop environment I believe it should already work. If you are using a more minimalist setup like WSL the easiest way to get OpenGL support is probably to install mesa-utils.

    sudo apt install mesa-utils

The rest of the dependencies are included as submodules and built during the same build step as the main executable. when cloning the repo you will need to clone recursively for this to work.

    git clone --recurse-submodules https://github.com/Andrew-Moody/numerical-analysis.git 

For the curious the included dependencies are: GLFW, GLAD, and Assimp. GLAD doesn't actually have a repo that can be submoduled you just generate files for your configuration using a web based tool. I've included a set of generated files with the required license. Hopefully there aren't any issues caused by incompatible configurations.

Next you will need to configure for building using CMake. We need to tell CMake where to look for files and where to output results. The simple way is to first navigate to the build folder located under the repo folder. from there execute the cmake command using the parent folder as the argument (../) which tells CMake to look in the repo folder (one directory up from current) and place configuration output in build (the current directory). [repo] is the path to the cloned repository (should be named numerical_analysis by default)

    cd [repo]/build
    cmake ../

If this is confusing the following command works without having to be in a specific directory first.

    cmake -B [repo]/build -S [repo]

Now you can actually build the project using make from the build directory (or use cmake --build [path to build directory]). It might take a bit of time to build the first time due to Assimp's size.

    cmake --build [repo]/build

And finally, to run the resulting executable you will need to first navigate to the folder it was built into otherwise file paths will not work correctly (looking for a work around for this).

    cd <repo>/build/numerical_analysis
    ./numerical_analysis
