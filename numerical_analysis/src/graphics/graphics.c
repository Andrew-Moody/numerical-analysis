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

void errorCallbackGLFW(int error, const char* description)
{
    printf(" GLFW_ERROR : %s\n", description);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Render a triangle to show everything works
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

    // A single triangle in normalized device coordinates (y is up unlike screen coords)
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f, 0.5f, 0.0f };

    struct Mesh triangle_mesh = { vertices, 9, NULL, 0 };

    struct Model triangle_model;
    model_init(&triangle_model, &triangle_mesh);


    // Create a grid of quads
    int x_steps = 25;
    int y_steps = 25;

    int grid_size = 3 * (x_steps + 1) * (y_steps + 1);

    float* grid_vertices = (float*)malloc(grid_size * sizeof(float));

    if (!grid_vertices)
    {
        return -1;
    }

    // Set vertices
    for (int j = 0; j <= y_steps; ++j)
    {
        for (int i = 0; i <= x_steps; ++i)
        {
            int index = 3 * (i + j * (x_steps + 1));

            grid_vertices[index] = -0.5f + (float)i / x_steps;
            grid_vertices[index + 1] = -0.5f + (float)j / y_steps;
            grid_vertices[index + 2] = 0.0f;
        }
    }

    int indices_size = 6 * x_steps * y_steps;

    unsigned int* grid_indices = (unsigned int*)malloc(indices_size * sizeof(unsigned int));

    if (!grid_indices)
    {
        return -1;
    }

    // Set indices
    for (int j = 0; j < y_steps; ++j)
    {
        for (int i = 0; i < x_steps; ++i)
        {
            int triangle_idx = 6 * (i + j * x_steps);

            int v1 = (i + j * (y_steps + 1));
            int v2 = v1 + 1;
            int v3 = (i + (j + 1) * (y_steps + 1));
            int v4 = v3 + 1;

            if (triangle_idx + 2 >= indices_size)
            {
                return -1;
            }

            // Note winding order
            grid_indices[triangle_idx] = v1;
            grid_indices[triangle_idx + 1] = v2;
            grid_indices[triangle_idx + 2] = v3;

            grid_indices[triangle_idx + 3] = v2;
            grid_indices[triangle_idx + 4] = v4;
            grid_indices[triangle_idx + 5] = v3;
        }
    }

    struct Mesh grid_mesh;

    grid_mesh.vertices = grid_vertices;
    grid_mesh.vertices_length = grid_size;
    grid_mesh.indices = grid_indices;
    grid_mesh.indices_length = indices_size;
    grid_vertices = NULL;
    grid_indices = NULL;

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
    shader_init(&shader, BASIC_VS_SRC, BASIC_FS_SRC);

    // Rendering calls will now use our own shader
    glUseProgram(shader);

    // Enable back face culling
    glEnable(GL_CULL_FACE);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Default

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe

    // Process Input and Rendering commands until a close event is recieved
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw Triangle
        //model_draw(&triangle_model);

        // Draw grid
        //model_draw(&grid_model);

        // Draw mesh
        model_draw(&stl_model);

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
