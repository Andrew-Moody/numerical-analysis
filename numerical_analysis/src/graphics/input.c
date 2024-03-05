#include "input.h"

#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "application.h"
#include "camera.h"

// Called when a key event is detected
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    // Unpack the application data from the user defined window payload
    struct Application* application = glfwGetWindowUserPointer(window);

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        printf("W Pressed\n");
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_REPEAT)
    {
        printf("W Repeated\n");
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
    {
        printf("W Released\n");
    }


    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        application->camera->proj_matrix = mat4_multiply(mat4_translation(0.f, 0.1f, 0.f), application->camera->proj_matrix);
    }

    if (key == GLFW_KEY_A && action == GLFW_PRESS)
    {
        application->camera->proj_matrix = mat4_multiply(mat4_translation(-0.1f, 0.f, 0.f), application->camera->proj_matrix);
    }

    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        application->camera->proj_matrix = mat4_multiply(mat4_translation(0.f, -0.1f, 0.f), application->camera->proj_matrix);
    }

    if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        application->camera->proj_matrix = mat4_multiply(mat4_translation(0.1f, 0.f, 0.f), application->camera->proj_matrix);
    }

    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
    {
        application->camera->proj_matrix = mat4_multiply(mat4_rotation(-5.0f, 0.0f, 0.0f), application->camera->proj_matrix);
    }

    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
    {
        application->camera->proj_matrix = mat4_multiply(mat4_rotation(0.0f, -5.0f, 0.0f), application->camera->proj_matrix);
    }

    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
    {
        application->camera->proj_matrix = mat4_multiply(mat4_rotation(5.0f, 0.0f, 0.0f), application->camera->proj_matrix);
    }

    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
    {
        application->camera->proj_matrix = mat4_multiply(mat4_rotation(0.0f, 5.0f, 0.0f), application->camera->proj_matrix);
    }

    glUniformMatrix4fv(application->proj_matrix_id, 1, GL_FALSE, (float*)&(application->camera->proj_matrix));
}

void input_init(struct GLFWwindow* window)
{
    glfwSetKeyCallback(window, key_callback);
}

// Continuous Polling
void process_input(struct GLFWwindow* window)
{
    struct Application* application = glfwGetWindowUserPointer(window);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        struct mat4 translation = mat4_translation(0.1f, 0.f, 0.f);

        //application->camera->proj_matrix = mat4_multiply(application->camera->proj_matrix, translation);
        application->camera->proj_matrix = mat4_multiply(translation, application->camera->proj_matrix);
    }



    glUniformMatrix4fv(application->proj_matrix_id, 1, GL_FALSE, (float*)&(application->camera->proj_matrix));
}