#pragma once

#include "transform.h"

struct Camera
{
    int screen_width;
    int screen_height;
    float near_z;
    float far_z;
    float zoom;
    struct mat4 proj_matrix;
    struct mat4 transform;
};


void camera_init(struct Camera* camera, int width, int height, float near, float far, float zoom);