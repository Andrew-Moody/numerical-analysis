#pragma once

#include "vector.h"

struct Mesh;

struct Node
{
    struct vec3 pos;
    struct vec3 force;
    struct vec3 moment;
    struct vec3 displacement;
    struct vec3 rotation;
};

struct Element
{
    int node1;
    int node2;
    float elastic_modulus;
    float shear_modulus;
    float radius;
};

struct Frame
{
    struct Node* nodes;
    struct Element* elements;
    int node_count;
    int element_count;

    // Material properties
    float elastic_modulus; // GPa
    float shear_modulus; // GPa
    float yield_strength; // GPa
};

//void structural(void);

void frame_solve(struct Frame* frame);

void frame_init(struct Frame* frame);

void frame_release(struct Frame* frame);

void frame_create_mesh(struct Frame* frame, struct Mesh* mesh);
