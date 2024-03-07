#pragma once

#include "transform.h"
#include "vector.h"

struct Camera
{
    int screen_width;
    int screen_height;
    float near_z;
    float far_z;
    float zoom;
    struct mat4 proj_matrix;
    struct mat4 transform;
    struct mat4 view_transform;
    struct vec3 position;
    struct vec3 rotation;
};


void camera_init(struct Camera* camera, int width, int height, float near, float far, float zoom);

void camera_update(struct Camera* camera);

void camera_translate(struct Camera* camera, float x, float y, float z);

void camera_rotate(struct Camera* camera, float x, float y, float z);