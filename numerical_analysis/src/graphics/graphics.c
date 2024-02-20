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
#include "camera.h"

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

    // Create the basic shader program
    GLuint shader;
    //shader_init(&shader, BASIC_VS_SRC, BASIC_FS_SRC);
    shader_init(&shader, VCOLOR_VS_SRC, VCOLOR_FS_SRC);

    // Rendering calls will now use our own shader
    glUseProgram(shader);

    // Get an id for the transform matrix in the basic shader
    GLuint transform_id = glGetUniformLocation(shader, "transform");

    // Get the id for the projection matrix
    GLuint projection_id = glGetUniformLocation(shader, "projection");


    struct Camera camera;
    camera_init(&camera, 800, 600, 0.f, 10.f, 4.0f);

    glUniformMatrix4fv(projection_id, 1, GL_FALSE, (float*)&camera.proj_matrix);

    // Create a grid of quads
    int x_steps = 25;
    int y_steps = 25;

    struct Mesh grid_mesh;
    create_grid_mesh(x_steps, y_steps, &grid_mesh);

    struct Model grid_model;
    model_init(&grid_model, &grid_mesh);
    grid_model.transform = MAT4_IDENTITY;
    grid_model.cull_backface = GL_TRUE;
    grid_model.draw_wireframe = GL_TRUE;

    // Load a mesh from STL file
    struct Mesh stl_mesh;
    //load_stl("../../models/3DBenchy.stl", &stl_mesh);
    load_stl("../../models/orientation.stl", &stl_mesh);

    printf("vert length: %i, index length: %i\n", stl_mesh.vertices_length, stl_mesh.indices_length);

    struct Model stl_model;
    model_init(&stl_model, &stl_mesh);
    stl_model.transform = mat4_transform(0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f);
    stl_model.cull_backface = GL_FALSE;
    stl_model.draw_wireframe = GL_TRUE;

    // Benchy model is too big to fit on screen unless perspective projection is used
    // also needed to be rotated and shifted down to center on the screen
    //stl_model.transform = mat4_transform(0.0f, -0.5f, 0.0f, -90.0f, 0.0f, 0.0f, 0.025f, 0.025f, 0.025f);


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

    // Call when finished with GLFW to release any remaining windows/resources
    glfwTerminate();

    return 0;
}
