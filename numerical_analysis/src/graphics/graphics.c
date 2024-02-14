#include "graphics.h"

#include <stdlib.h>
#include <stdio.h>

// glad must be included before glfw
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "model.h"

void errorCallbackGLFW(int error, const char *description)
{
    printf(" GLFW_ERROR : %s\n", description);
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Render a triangle to show everything works
int graphics_test(void)
{
    loadModel("../../models/3DBenchy.stl");

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
    GLFWwindow *window = glfwCreateWindow(800, 600, "A GLFW Window", NULL, NULL);
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
        0.0f, 0.5f, 0.0f};

    // The unique ID that identifies a Vertex Buffer Object
    GLuint vbo;
    GLsizei numBuffers = 1; // Only creating a single buffer in this case

    // Generates 1 or more VBO ids
    glGenBuffers(numBuffers, &vbo);

    // generate a Vertex Array Object id
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    // While bound, Vertex Array Object stores information about the bound Vertex Buffer Object
    // as calls are made to modify properties on the VBO

    // Binding the buffer to the array buffer target
    // any calls targeting array buffer will effect vbo until it is unbound
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Copy vertex data to gpu memory
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Tell the gpu how the vertex buffer data is laid out (vbo is still bound)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // vao is unbound and now remembers info about the vbo
    // bind the vao again before drawing
    glBindVertexArray(0);


    // Create a grid of quads
    int x_steps = 25;
    int y_steps = 25;

    int grid_size = 3 * (x_steps + 1) * (y_steps + 1);

    float *grid = (float *)malloc(grid_size * sizeof(float));

    if (!grid)
    {
        return -1;
    }

    // Set vertices
    for (int j = 0; j <= y_steps; ++j)
    {
        for (int i = 0; i <= x_steps; ++i)
        {
            int index = 3 * (i + j * (x_steps + 1));

            grid[index] = -0.5f + (float)i / x_steps;
            grid[index + 1] = -0.5f + (float)j / y_steps;
            grid[index + 2] = 0.0f;
        }
    }

    int indices_size = 6 * x_steps * y_steps;

    unsigned int *grid_indices = (unsigned int *)malloc(indices_size * sizeof(unsigned int));

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

    GLuint grid_vao;
    glGenVertexArrays(1, &grid_vao);
    glBindVertexArray(grid_vao);

    GLuint grid_vbo;
    glGenBuffers(1, &grid_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, grid_vbo);
    glBufferData(GL_ARRAY_BUFFER, grid_size * sizeof(float), grid, GL_STATIC_DRAW);

    GLuint grid_ebo;
    glGenBuffers(1, &grid_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grid_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size * sizeof(unsigned int), grid_indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    free(grid);
    free(grid_indices);

    grid = NULL;
    grid_indices = NULL;

    // Basic vertex shader
    const char *vertexShaderSource = "#version 420 core\n"
                                     "layout (location = 0) in vec3 aPos;\n"
                                     "void main() { gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0); }\0";

    GLuint vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("Vertex Shader compilation failed: %s\n", infoLog);
    }

    // Basic fragment shader
    const char *fragmentShaderSource = "#version 420 core\n"
                                       "out vec4 FragColor;\n"
                                       "void main() { FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f); }\0";

    GLuint fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("Fragment Shader compilation failed: %s\n", infoLog);
    }

    // Link the vertex and fragment shaders into a shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(fragmentShader, GL_LINK_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("Shader linking failed: %s\n", infoLog);
    }

    // Shaders aren't needed after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Rendering calls will now use our own shader
    glUseProgram(shaderProgram);

    glEnable(GL_CULL_FACE); // Enable back face culling

    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Default

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Process Input and Rendering commands until a close event is recieved
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //glBindVertexArray(vao);
        //glDrawArrays(GL_TRIANGLES, 0, 3);

        // Draw grid
        glBindVertexArray(grid_vao);
        glDrawElements(GL_TRIANGLES, indices_size, GL_UNSIGNED_INT, 0); // Must use unsigned int here

        glfwSwapBuffers(window);
    }

    // Destroy a specific window
    glfwDestroyWindow(window);

    // Call when finished with GLFW should release any remaining windows/resources
    glfwTerminate();

    return 0;
}
