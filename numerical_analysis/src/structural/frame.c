#include "frame.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "vector.h"


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


void add_local_stiffness(float* k_global, float* k_local, int node1, int node2, int node_count)
{
    int offset = 6 * node1 + 36 * (node2 * node_count);

    for (int j = 0; j < 6; ++j)
    {
        for (int i = 0; i < 6; ++i)
        {
            int local_idx = i + j * 6;

            int global_idx = i + offset + j * 6 * node_count;

            k_global[global_idx] += k_local[local_idx];
        }
    }
}


void structural(void)
{
    printf("test\n");

    // Material properties for 1040 mild steel annealed (13 C)
    const float elastic_modulus = 200.f; // GPa
    const float shear_modulus = 80.f; // GPa
    const float yield_strength = 0.415f; // GPa, 415 MPa

    int node_count = 2;

    int element_count = 1;

    // it's also recommended to not cast the result of malloc as that could hide errors and is not necessary. C++ is 
    // different because void* will not implicitly convert and needs a cast but use of malloc in C++ is a special case

    // using sizeof on the dereferenced pointer keeps the type coupled to the type of the pointer so no need to
    // change the sizeof expression if the pointer type changes and will give an error if the variable name changes
    // sizeof(*ptr) is evaluated at compile time so the pointer is not actually dereferenced and is fine to be NULL

    // it is also sometimes recommended to put the sizeof expression first since this insures multiplications are
    // done with size_t. for example if width and height are int on a 64 bit system width * height * sizeof(something)
    // could overflow the temp int result of width * height but sizeof(something) * width * height will not unless it would
    // overflow size_t regardless

    struct Node* nodes = NULL;
    size_t nodes_size = sizeof(*nodes) * node_count;
    nodes = malloc(nodes_size);

    struct Element* elements = NULL;
    size_t elements_size = sizeof(*elements) * element_count;
    elements = malloc(elements_size);

    // Define node positions
    nodes[0].pos = (struct vec3){ 0.f, 0.f, 0.f };
    nodes[1].pos = (struct vec3){ 1.f, 0.f, 0.f };

    // Define boundary conditions (node1 is fixed, node2 has a force applied)
    nodes[0].displacement = (struct vec3){ 0.f, 0.f, 0.f };
    nodes[0].rotation = (struct vec3){ 0.f, 0.f, 0.f };
    nodes[1].force = (struct vec3){ 1.f, 0.f, 0.f };

    // Define an element between node1 and node2
    elements[0] = (struct Element){ 0, 1, elastic_modulus, shear_modulus, 1.f };


    float* k_global = malloc(sizeof(*k_global) * 36 * node_count * node_count);

    for (int i = 0; i < 36 * node_count * node_count; ++i)
    {
        k_global[i] = 0;
    }

    for (int element_idx = 0; element_idx < element_count; ++element_idx)
    {
        int node1 = elements[element_idx].node1;
        int node2 = elements[element_idx].node2;

        float length = vec3_distance(nodes[node1].pos, nodes[node2].pos);
        float area = elements[element_idx].radius * elements[element_idx].radius * 3.14159f;
        float k = elements[element_idx].elastic_modulus * area / length;

        // Fx = k * Ux

        // At each node there are 3 forces and 3 moments that produce 3 displacements and 3 rotations
        // we can build a 6x6 matrix that represents the relationship between the forces and moments
        // at one node and the displacements and rotations at another
        // each element will produce 4 of these 6x6 matrices one for each combination
        // These 6x6 matrices are then slotted in to the global matrix after transforming to global frame
        // The are pretty simple for axial stiffness only

        // kxy is the stiffness matrix for force on node x due to the displacement of node y
        float k11[36] = {
            k, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
        };

        float k12[36] = {
            -k, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
        };

        float k21[36] = {
            -k, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
        };

        float k22[36] = {
            k, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
        };

        // Will need to transform local to global space here

        // Add the local contributions to the global stiffness matrix
        add_local_stiffness(k_global, k11, node1, node1, node_count);
        add_local_stiffness(k_global, k12, node1, node2, node_count);
        add_local_stiffness(k_global, k21, node2, node1, node_count);
        add_local_stiffness(k_global, k22, node2, node2, node_count);
    }


    for (int j = 0; j < 6 * node_count; ++j)
    {
        for (int i = 0; i < 6 * node_count; ++i)
        {
            printf("%2.0f ", k_global[i + j * 6 * node_count]);
        }

        printf("\n");
    }


    free(k_global);
    free(nodes);
    free(elements);
}