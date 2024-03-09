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
    cd <the folder you cloned into>/build
    cmake ../
    make
    ./numerical_analysis/numerical_analysis

#### More Details
Ubuntu comes with gcc which should support OpenMP out of the box. You just need to have the -fopenmp compiler flag set which should be handled correctly with the provided CMake files.

You will need to have CMake installed to build. I tried to keep the minimum version reasonable but there are a few features that require 3.13 or above. If the version of ubuntu you have is recent it should come with a high enough version to install directly. It is recommended to check for package updates first.

    sudo apt update
    sudo apt install cmake

You will also need to have OpenGL support for graphics. If you use a desktop environment I believe it should already work. If you are using a more minimalist setup like WSL the easiest way to get OpenGL support is probably to install mesa-utils.

    sudo apt install mesa-utils

The rest of the dependencies are included as submodules and built during the same build step as the main executable. when cloning the repo you will need to clone recursively for this to work.

    git clone --recurse-submodules https://github.com/Andrew-Moody/numerical-analysis.git 

For the curious the included dependencies are: GLFW, GLAD, and Assimp

Next you will need to configure for building using CMake. first navigate the folder the repo was cloned into and the cd into the build folder. from there execute the cmake command using the parent folder as the argument (../)

    cd <the folder you cloned into>/build
    cmake ../

Now you can actually build the project using make from the build directory

    make

And finally run the resulting executable.

    ./numerical_analysis/numerical_analysis