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


void add_local_stiffness(float* k_global, float* k_local, struct mat3 transform, int node1, int node2, int node_count)
{
    // Inverse transformation matrix is its transpose
    struct mat3 transinv = mat3_transpose(transform);

    float transform6x6[36];

    for (int j = 0; j < 6; ++j)
    {
        for (int i = 0; i < 6; ++i)
        {
            transform6x6[i + j * 6] = 0;
        }
    }

    transform6x6[0] = transform.e11;
    transform6x6[1] = transform.e12;
    transform6x6[2] = transform.e13;

    transform6x6[6] = transform.e21;
    transform6x6[7] = transform.e22;
    transform6x6[8] = transform.e23;

    transform6x6[12] = transform.e31;
    transform6x6[13] = transform.e32;
    transform6x6[14] = transform.e33;

    transform6x6[21] = transform.e11;
    transform6x6[22] = transform.e12;
    transform6x6[23] = transform.e13;

    transform6x6[27] = transform.e21;
    transform6x6[28] = transform.e22;
    transform6x6[29] = transform.e23;

    transform6x6[33] = transform.e31;
    transform6x6[34] = transform.e32;
    transform6x6[35] = transform.e33;

    float invtrans6x6[36];
    for (int i = 0; i < 36; ++i)
    {
        invtrans6x6[i] = transform6x6[i];
    }

    mat6_transpose(invtrans6x6);

    float kxt[36];
    mat6_multiply(kxt, k_local, transform6x6);
    float k_g[36];
    mat6_multiply(k_g, invtrans6x6, kxt);

    int offset = 6 * node1 + 36 * (node2 * node_count);

    for (int j = 0; j < 6; ++j)
    {
        for (int i = 0; i < 6; ++i)
        {
            int local_idx = i + j * 6;

            int global_idx = i + offset + j * 6 * node_count;

            k_global[global_idx] += k_g[local_idx];
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
    nodes[1].pos = (struct vec3){ 0.f, 1.f, 0.f };

    // Define boundary conditions (node1 is fixed, node2 has a force applied)
    nodes[0].displacement = (struct vec3){ 0.f, 0.f, 0.f };
    nodes[0].rotation = (struct vec3){ 0.f, 0.f, 0.f };
    nodes[1].force = (struct vec3){ 1.f, 1.f, 1.f };

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

        // The stiffness matrices above assume the element is lying in the global reference frame
        // with the length oriented along the global x axis
        // Of course the elements can be in any orientation so the global displacements do not relate
        // to global forces via these stiffness matrices but instead relate in the local frame
        // ie. F_l = K * U_l

        // If we have a matrix T that represents the transformation from global to local frame
        // we can transform the forces and displacements F_l = T * F_g, U_l = T * U_g
        // relating via local stiffness K gives T * F_g = K * T * U_g
        // Pre-multiplying both sides by T inverse gives F_g = T^-1 * K * T * U_g
        // From here it can be seen that the stiffness matrix that relates global displacements
        // to global forces is K_g = T^-1 * K * T
        // so before contributing the elements stiffness to the global stiffness matrix
        // we must first find the global to local transformation matrix T for the element

        // first subtract global position of node1 from global position of node2 to give
        // a global vector that lies along the length of the element (so along local x axis)
        // Most likely an element in the frame is not rotated about local x when in an unstressed position
        // so it would be fine to assume local theta_x is 0 but a boundary condition could impose a rotation about x

        // If we do make the assumption that theta_x is zero then we know that a global vector pointing in the global y direction
        // is incident on the local x-y plane of the element and likewise a global vector in the z direction is incident
        // on the local x-z plane so we can use either as a starting point we just need one where the magnitude of the crossproduct
        // local x axis is not close enough to zero to cause floating point errors when normalizing
        // this can be checked easily by checking if the dot product magnitude is close to 1 and picking the other vector if so

        // normalize the element axial vector to give the local x axis in global frame
        struct vec3 x_axis = vec3_subtract(nodes[node2].pos, nodes[node1].pos);
        x_axis = vec3_normalize(x_axis);

        // Default initialize y and x axis
        struct vec3 y_axis = { 0.f, 0.f, 0.f };
        struct vec3 z_axis = { 0.f, 0.f, 0.f };

        // We need a second vector that is not to close to parallel with local x
        // to produce a cross product with reasonable magnitude
        // if the dot product magnitude between local x and global y is small
        // then they are close to alignment (regardless of direction) and global z
        // becomes a better choice
        float dot = abs(vec3_dot(x_axis, (struct vec3) { 0.f, 1.f, 0.f }));
        printf("dot: %f\n", dot);
        if (dot < 0.5f)
        {
            // local y is reasonably close to global y so the local z axis in global frame
            // is the normalized cross product between the local x axis in global frame and the global y axis
            z_axis = vec3_cross(x_axis, (struct vec3) { 0.f, 1.f, 0.f });
            z_axis = vec3_normalize(z_axis);

            // Now local y is just cross product of local z and local x
            y_axis = vec3_cross(z_axis, x_axis);
            y_axis = vec3_normalize(y_axis); // should be unit length already but might as well make sure

            printf("x_axis: %2.f, %2.f, %2.f\n", x_axis.x, x_axis.y, x_axis.z);
            printf("y_axis: %2.f, %2.f, %2.f\n", y_axis.x, y_axis.y, y_axis.z);
            printf("z_axis: %2.f, %2.f, %2.f\n", z_axis.x, z_axis.y, z_axis.z);
        }
        else
        {
            // local z is reasonably close to global z so the local y axis in global frame
            // is the normalized cross product between the global z axis and the local x axis in global frame
            // (could have also done vec3_cross(x_axis, (struct vec3) { 0.f, 0.f, -1.f }) just comfirm with right hand rule)
            y_axis = vec3_cross((struct vec3) { 0.f, 0.f, 1.f }, x_axis);

            printf("z_axis: %2.1f, %2.1f, %2.1f\nn", z_axis.x, z_axis.y, z_axis.z);

            y_axis = vec3_normalize(y_axis);

            // Now local z is just cross product of local x and local y
            z_axis = vec3_cross(x_axis, y_axis);
            z_axis = vec3_normalize(z_axis); // should be unit length already but might as well make sure

            printf("x_axis: %2.f, %2.f, %2.f\n", x_axis.x, x_axis.y, x_axis.z);
            printf("y_axis: %2.f, %2.f, %2.f\n", y_axis.x, y_axis.y, y_axis.z);
            printf("z_axis: %2.f, %2.f, %2.f\n", z_axis.x, z_axis.y, z_axis.z);
        }

        // Build the transformation matrix from global to local for this element
        struct mat3 transform = {
            x_axis.x, x_axis.y, x_axis.z,
            y_axis.x, y_axis.y, y_axis.z,
            z_axis.x, z_axis.y, z_axis.z,
        };


        /* printf("x_axis: %2.f, %2.f, %2.f\n", x_axis.x, x_axis.y, x_axis.z);
        printf("y_axis: %2.f, %2.f, %2.f\n", y_axis.x, y_axis.y, y_axis.z);
        printf("z_axis: %2.f, %2.f, %2.f\n", z_axis.x, z_axis.y, z_axis.z); */

        // Add the local contributions to the global stiffness matrix
        add_local_stiffness(k_global, k11, transform, node1, node1, node_count);
        add_local_stiffness(k_global, k12, transform, node1, node2, node_count);
        add_local_stiffness(k_global, k21, transform, node2, node1, node_count);
        add_local_stiffness(k_global, k22, transform, node2, node2, node_count);
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