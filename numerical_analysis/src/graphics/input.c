#include "input.h"

#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "application.h"
#include "camera.h"


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{

}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // There is a bit of an issue i'm having trouble finding information on
    // and am not sure if it is a problem with GLFW, mouse settings or the mouse itself
    // When you are scrolliing in one direction and then change to a different direction
    // one "click" of the scroll wheel does not get registered (no callback is called)
    // so if scrolling down the first upward click does not register and vice versa
    // When using the mouse normally say in VSCode or web browser this skip does not happen

    // Luckily it gives a delta rather than absolute position
    //printf("Scroll: (%f, %f)\n", xoffset, yoffset);

    // Check for scroll up or down
    /* if (yoffset < -0.5f)
    {
        printf("Scroll down\n");
    }
    else if (yoffset > 0.5f)
    {
        printf("Scroll up\n");
    }
    else
    {
        printf("No Scroll\n");
    } */

    // Unpack the application data from the user defined window payload
    struct Application* application = glfwGetWindowUserPointer(window);
    if (application)
    {
        //printf("Zoom: %f\n", application->camera->zoom);

        float sensitivity = 0.5f;
        application->camera->zoom -= yoffset * sensitivity;
    }


}


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
        printf("Space Pressed\n");
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_REPEAT)
    {
        printf("Space Repeated\n");
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
    {
        printf("Space Released\n");
    }


    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        camera_translate(application->camera, 0.0f, 0.0f, 0.1f);
    }

    if (key == GLFW_KEY_A && action == GLFW_PRESS)
    {
        camera_translate(application->camera, -0.1f, 0.0f, 0.0f);
    }

    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        camera_translate(application->camera, 0.0f, 0.0f, -0.1f);
    }

    if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        camera_translate(application->camera, 0.1f, 0.0f, 0.0f);
    }

    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
    {
        camera_rotate(application->camera, -5.0f, 0.0f, 0.0f);
    }

    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
    {
        camera_rotate(application->camera, 0.0f, -5.0f, 0.0f);
    }

    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
    {
        camera_rotate(application->camera, 5.0f, 0.0f, 0.0f);
    }

    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
    {
        camera_rotate(application->camera, 0.0f, 5.0f, 0.0f);
    }
}

void input_init(struct GLFWwindow* window)
{
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
}

// Continuous Polling
void process_input(struct GLFWwindow* window)
{
    struct Application* application = glfwGetWindowUserPointer(window);

    // Check if Left Mouse is currently held down
    static int mouse_left_pressed = GLFW_RELEASE;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        static double prev_x = 0.f;
        static double prev_y = 0.f;

        double curr_x;
        double curr_y;

        glfwGetCursorPos(window, &curr_x, &curr_y);

        // Check if button was pressed this frame (or since last poll)
        if (mouse_left_pressed == GLFW_RELEASE)
        {
            mouse_left_pressed = GLFW_PRESS;

            prev_x = curr_x;
            prev_y = curr_y;
        }

        double delta_x = curr_x - prev_x;
        double delta_y = prev_y - curr_y; // Switch top left to bottom left

        float pan_x = delta_x / 75.f;
        float pan_y = delta_y / 75.f;

        prev_x = curr_x;
        prev_y = curr_y;

        camera_translate(application->camera, pan_x, pan_y, 0.0f);
    }
    else if (mouse_left_pressed == GLFW_PRESS)
    {
        // Mouse was released since the last poll
        mouse_left_pressed = GLFW_RELEASE;
    }


    // Check if Right Mouse is currently held down
    static int mouse_right_pressed = GLFW_RELEASE;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {

        static double prev_x = 0.f;
        static double prev_y = 0.f;

        double curr_x;
        double curr_y;

        glfwGetCursorPos(window, &curr_x, &curr_y);

        // Check if button was pressed this frame (or since last poll)
        if (mouse_right_pressed == GLFW_RELEASE)
        {
            mouse_right_pressed = GLFW_PRESS;

            prev_x = curr_x;
            prev_y = curr_y;
        }

        double delta_x = curr_x - prev_x;
        double delta_y = curr_y - prev_y;

        float rot_x = 15.f * delta_y / 75.f;
        float rot_y = 15.f * delta_x / 75.f;

        prev_x = curr_x;
        prev_y = curr_y;

        camera_rotate(application->camera, rot_x, rot_y, 0.0f);
    }
    else if (mouse_right_pressed == GLFW_PRESS)
    {
        // Mouse was released since the last poll
        mouse_right_pressed = GLFW_RELEASE;
    }
}