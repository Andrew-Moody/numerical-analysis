#pragma once

#include "vector.h"
#include "matrix.h"

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

struct EquationSet
{
    struct Matrix stiffness;
    struct Matrix stiff_bc;
    struct vecf forces;
    struct vecf displacements;
};

// Build a set of matrices and vectors representing the problem
void frame_build_equations(struct Frame* frame, struct EquationSet* eqset);

// Populate per node properties using displacements to back calculate forces
void frame_update_results(struct Frame* frame, struct EquationSet* eqset);

// Frees resources held by the frame
void frame_release(struct Frame* frame);

// Create a graphical mesh representation of the frame
void frame_create_mesh(struct Frame* frame, struct Mesh* mesh);

void equationset_release(struct EquationSet* eqset);

void frame_print_results(struct Frame* frame);

// Solve the linear equation representing the frame using an iterative method with OpenMP
void frame_solve(struct Frame* frame);