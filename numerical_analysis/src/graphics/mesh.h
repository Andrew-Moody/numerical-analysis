#pragma once

struct Mesh
{
    float* vertices;
    unsigned int vertices_length;
    unsigned int* indices;
    unsigned int indices_length;
};

void mesh_release(struct Mesh** mesh);