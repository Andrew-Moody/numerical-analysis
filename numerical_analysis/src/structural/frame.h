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

enum BoundaryKind
{
    BC_Default = 0,
    BC_Force,
    BC_Moment,
    BC_Displacement,
    BC_Rotation
};

struct BoundaryCondition
{
    int node; // The node the condition applies to
    enum BoundaryKind kind; // What property the boundary condition applies to (force, displacement, etc.)
    struct vec3 value; // The value of the property being fixed
};

struct Frame
{
    struct Node* nodes;
    struct Element* elements;
    struct BoundaryCondition* bconditions;
    int node_count;
    int element_count;
    int bc_count;

    // Material properties
    float elastic_modulus; // GPa
    float shear_modulus; // GPa
    float yield_strength; // GPa
};


void frame_solve(struct Frame* frame);

// Creates a hardcoded frame for testing. Use frame_import instead to load from a file
void frame_init(struct Frame* frame);

void frame_release(struct Frame* frame);

void frame_create_mesh(struct Frame* frame, struct Mesh* mesh);
