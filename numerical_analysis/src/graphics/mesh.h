#pragma once

struct Vertex
{
    float x, y, z; // Local Position
    float r, g, b; // vertex Color
};


struct Mesh
{
    struct Vertex* vertices;
    unsigned int vertices_length;
    unsigned int* indices;
    unsigned int indices_length;
};

void mesh_release(struct Mesh** mesh);