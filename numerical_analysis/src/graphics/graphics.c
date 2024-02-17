#include "graphics.h"

#include <stdlib.h>
#include <stdio.h>

// glad must be included before glfw
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "import.h"
#include "shader.h"
#include "model.h"
#include "mesh.h"
#include "grid.h"
#include "matrix.h"

void errorCallbackGLFW(int error, const char* description)
{
    printf(" GLFW_ERROR : %s\n", description);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Render a model imported from an STL file or a generated grid of quads
int graphics_test(void)
{
    // Few functions can be called before glfwInit
    // setting the error callback now ensures it gets called for errors during init
    glfwSetErrorCallback(errorCallbackGLFW);

    if (!glfwInit())
    {
        printf("GLFW failed to initialize\n");
    }

    // Specify the version of OpenGL you wish to use (wsl ubuntu came with 4.2)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a new window (also crates an OpenGL context)
    GLFWwindow* window = glfwCreateWindow(800, 600, "A GLFW Window", NULL, NULL);
    if (!window)
    {
        printf("GLFW failed to create a window\n");
        glfwTerminate();
        return -1;
    }

    // Set the context owned by window as the current context
    glfwMakeContextCurrent(window);

    // Use GLAD to load OpenGL function pointers
    // GLFW handles address location for specific operating system
    // must be called after a context is set
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("GLAD failed to load OpenGL\n");
        glfwTerminate();
        return -1;
    }

    // Set the viewport dimensions
    glViewport(0, 0, 800, 600);

    // Calls glViewport() when a resize event is recieved
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // How many frames to wait between buffer swaps (1 essentially acts as VSYNC)
    // setting to 0 removes the limit allowing the framerate to be uncapped
    glfwSwapInterval(1);

    // Create a grid of quads
    int x_steps = 25;
    int y_steps = 25;

    struct Mesh grid_mesh;
    create_grid_mesh(x_steps, y_steps, &grid_mesh);

    struct Model grid_model;
    model_init(&grid_model, &grid_mesh);


    // Load a mesh from STL file
    struct Mesh stl_mesh;
    load_stl("../../models/3DBenchy.stl", &stl_mesh);

    printf("vert length: %i, index length: %i\n", stl_mesh.vertices_length, stl_mesh.indices_length);

    struct Model stl_model;
    model_init(&stl_model, &stl_mesh);


    // Create the basic shader program
    GLuint shader;
    //shader_init(&shader, BASIC_VS_SRC, BASIC_FS_SRC);
    shader_init(&shader, VCOLOR_VS_SRC, VCOLOR_FS_SRC);

    // Rendering calls will now use our own shader
    glUseProgram(shader);

    // Get an id for the transform matrix in the basic shader
    GLuint transform_id = glGetUniformLocation(shader, "transform");

    // Original model is too big to fit on screen unless perspective projection is used
    // also needed to be rotated and shifted down to center on the screen
    struct mat4 scale = mat4_scale(0.025f, 0.025f, 0.025f);
    struct mat4 rotation = mat4_rotation(-90.0f, 0.0f, 0.0f);
    struct mat4 translation = mat4_translation(0.0f, -0.5f, 0.0f);

    // Multiplication order is Scale * Rot * Trans because the multiplication
    // function is in terms of a row major matrix. May decide to change this around
    struct mat4 transform = mat4_multiply(rotation, translation);
    transform = mat4_multiply(scale, transform);

    stl_model.transform = transform;


    // Enable back face culling
    glEnable(GL_CULL_FACE);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Default

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe

    // Process Input and Rendering commands until a close event is recieved
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw grid
        //model_draw(&grid_model, transform_id);

        // Draw mesh
        model_draw(&stl_model, transform_id);

        glfwSwapBuffers(window);
    }

    free(grid_mesh.vertices);
    free(grid_mesh.indices);

    free(stl_mesh.vertices);
    free(stl_mesh.indices);

    // Destroy a specific window
    glfwDestroyWindow(window);

    // Call when finished with GLFW should release any remaining windows/resources
    glfwTerminate();

    return 0;
}
