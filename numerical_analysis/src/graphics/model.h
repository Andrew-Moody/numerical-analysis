#pragma once

#include "transform.h"

struct Mesh;

struct Model
{
    unsigned int vertex_array_obj;
    unsigned int vertex_buffer_obj;
    unsigned int element_buffer_obj;
    struct Mesh* mesh;
    struct mat4 transform;
    int cull_backface;
    int draw_wireframe;
};

void model_init(struct Model* model, struct Mesh* mesh);

void model_draw(struct Model* model, unsigned int transform_id);

void model_release(struct Model* model);
