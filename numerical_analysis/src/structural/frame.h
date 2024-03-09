#pragma once

#include "vector.h"

struct Mesh;

struct Node
{
    // The position of the node in 3D space
    struct vec3 pos;

    // Properties of the node we want to solve for
    struct vec3 force;
    struct vec3 moment;
    struct vec3 displacement;
    struct vec3 rotation;
};

struct Element
{
    // Indices into the node array for the nodes at the ends of the element
    int node1;
    int node2;

    // Material / structural properties
    // Material properties
    float elastic_modulus; // GPa
    float shear_modulus; // GPa
    float radius; // meters
    //float yield_strength; // GPa
};

// Used to specify which property a boundary condition affects
enum BoundaryKind
{
    BC_Default = 0,
    BC_Force,
    BC_Moment,
    BC_Displacement,
    BC_Rotation,
    BC_Joint
};

struct BoundaryCondition
{
    int node; // The node the condition applies to
    enum BoundaryKind kind; // What property the boundary condition applies to (force, displacement, etc.)
    struct vec3 value; // The value of the property being fixed
};

struct Frame
{
    // Arrays of nodes, elements, and boundary conditions that make up a frame definition
    struct Node* nodes;
    struct Element* elements;
    struct BoundaryCondition* bconditions;
    int node_count;
    int element_count;
    int bc_count;
};

// Solve the linear equation representing the frame using an iterative method
void frame_solve(struct Frame* frame);

// Frees resources held by the frame
void frame_release(struct Frame* frame);

// Create a graphical mesh representation of the frame
void frame_create_mesh(struct Frame* frame, struct Mesh* mesh);
