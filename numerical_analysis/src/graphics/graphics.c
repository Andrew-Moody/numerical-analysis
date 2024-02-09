#include "graphics.h"

#include <stdio.h>

// glad must be included before glfw
#include "glad/glad.h"
#include "GLFW/glfw3.h"

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

    // Process Input and Rendering commands until a close event is recieved
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
    }

    // Destroy a specific window
    glfwDestroyWindow(window);

    // Call when finished with GLFW should release any remaining windows/resources
    glfwTerminate();

    return 0;
}
