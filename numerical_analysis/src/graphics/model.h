#pragma once

struct Mesh;

struct Model
{
    unsigned int vertex_array_obj;
    unsigned int vertex_buffer_obj;
    unsigned int element_buffer_obj;
    struct Mesh* mesh;
};

void model_init(struct Model* model, struct Mesh* mesh);

void model_draw(struct Model* model);

void model_release(struct Model** model);
