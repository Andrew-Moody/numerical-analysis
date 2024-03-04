#include "frame.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "vector.h"
#include "matrix.h"
#include "linearsolve.h"
#include "mesh.h"


void transform_element_stiffness(float* k_local, struct mat3 transform);
void add_element_stiffness(float* k_global, float* k_element, struct mat3 transform, int node1, int node2, int node_count);

void frame_solve(struct Frame* frame)
{
    int dof_per_node = 6;
    int dof_count = dof_per_node * frame->node_count;

    // Alocate memory and initialize to zero for the forces, displacements, and stiffness matrix
    float* k_global = malloc(sizeof(*k_global) * dof_count * dof_count);

    for (int i = 0; i < dof_count * dof_count; ++i)
    {
        k_global[i] = 0;
    }

    float* displacements = malloc(sizeof(*displacements) * dof_count);
    float* forces = malloc(sizeof(*forces) * dof_count);

    for (int i = 0; i < dof_count; ++i)
    {
        displacements[i] = 0.f;
        forces[i] = 0.f;
    }

    // Add each elements contribution to the stiffness matrix
    for (int element_idx = 0; element_idx < frame->element_count; ++element_idx)
    {
        int node1 = frame->elements[element_idx].node1;
        int node2 = frame->elements[element_idx].node2;

        float length = vec3_distance(frame->nodes[node1].pos, frame->nodes[node2].pos);
        float area = frame->elements[element_idx].radius * frame->elements[element_idx].radius * 3.14159f;
        float k = frame->elements[element_idx].elastic_modulus * area / length;

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
        // on the local x-z plane so we can use either as a starting point we just need one where the magnitude of the cross product
        // with the local x axis is not close enough to zero to cause floating point errors when normalizing
        // this can be checked easily by checking if the dot product magnitude is close to 1 and picking the other vector if so

        // normalize the element axial vector to give the local x axis in global frame
        struct vec3 x_axis = vec3_subtract(frame->nodes[node2].pos, frame->nodes[node1].pos);
        x_axis = vec3_normalize(x_axis);

        // Default initialize y and x axis
        struct vec3 y_axis = { 0.f, 0.f, 0.f };
        struct vec3 z_axis = { 0.f, 0.f, 0.f };

        // We need a second vector that is not to close to parallel with local x
        // to produce a cross product with reasonable magnitude
        // if the dot product magnitude between local x and global y is small
        // then they are close to alignment (regardless of direction) and global z
        // becomes a better choice
        if (abs(vec3_dot(x_axis, (struct vec3) { 0.f, 1.f, 0.f })) < 0.5f)
        {
            // local y is reasonably close to global y so the local z axis in global frame
            // is the normalized cross product between the local x axis in global frame and the global y axis
            z_axis = vec3_cross(x_axis, (struct vec3) { 0.f, 1.f, 0.f });
            z_axis = vec3_normalize(z_axis);

            // Now local y is just cross product of local z and local x
            y_axis = vec3_cross(z_axis, x_axis);
            y_axis = vec3_normalize(y_axis); // should be unit length already but might as well make sure
        }
        else
        {
            // local z is reasonably close to global z so the local y axis in global frame
            // is the normalized cross product between the global z axis and the local x axis in global frame
            // (could have also done vec3_cross(x_axis, (struct vec3) { 0.f, 0.f, -1.f }) just comfirm with right hand rule)
            y_axis = vec3_cross((struct vec3) { 0.f, 0.f, 1.f }, x_axis);
            y_axis = vec3_normalize(y_axis);

            // Now local z is just cross product of local x and local y
            z_axis = vec3_cross(x_axis, y_axis);
            z_axis = vec3_normalize(z_axis); // should be unit length already but might as well make sure
        }

        // Build the transformation matrix from global to local for this element
        struct mat3 transform = {
            x_axis.x, x_axis.y, x_axis.z,
            y_axis.x, y_axis.y, y_axis.z,
            z_axis.x, z_axis.y, z_axis.z,
        };


        printf("x_axis: %2.f, %2.f, %2.f\n", x_axis.x, x_axis.y, x_axis.z);
        printf("y_axis: %2.f, %2.f, %2.f\n", y_axis.x, y_axis.y, y_axis.z);
        printf("z_axis: %2.f, %2.f, %2.f\n\n", z_axis.x, z_axis.y, z_axis.z);

        // Add the local contributions to the global stiffness matrix
        add_element_stiffness(k_global, k11, transform, node1, node1, frame->node_count);
        add_element_stiffness(k_global, k12, transform, node1, node2, frame->node_count);
        add_element_stiffness(k_global, k21, transform, node2, node1, frame->node_count);
        add_element_stiffness(k_global, k22, transform, node2, node2, frame->node_count);
    }


    for (int j = 0; j < dof_count; ++j)
    {
        for (int i = 0; i < dof_count; ++i)
        {
            printf("%2.0f ", k_global[i + j * dof_count]);
        }

        printf("\n");
    }

    printf("\n\n");


    // Need to make a copy of k_global before modifying it to add boundary conditions as it will be used later
    float* k_boundary = malloc(sizeof(*k_boundary) * dof_count * dof_count);

    for (int i = 0; i < dof_count * dof_count; ++i)
    {
        k_boundary[i] = k_global[i];
    }

    // Add boundary conditions

    // Fix displacement to zero for some nodes
    int fixed_count = 3;
    int fixed_nodes[3] = { 1, 2, 3 };

    // Apply a force at node2
    int node_forced = 0;


    for (int n = 0; n < fixed_count; ++n)
    {
        // Set the global stiffness rows and columns for Ux, Uy, Uz to 1 on diagonal and 0 everywhere else
        // for the node that is fixed (setting diagonals to 1 is not strictly necessary but facilitates some solution methods)
        int node = fixed_nodes[n];
        int rowstart = node * dof_per_node * dof_count;
        int colstart = node * dof_per_node;

        for (int j = 0; j < 6; ++j)
        {
            for (int i = 0; i < dof_count; ++i)
            {
                k_boundary[rowstart + i + j * dof_count] = 0;

                k_boundary[colstart + j + i * dof_count] = 0;
            }

            k_boundary[rowstart + colstart + dof_count * j + j] = 1;
        }
    }

    printf("Stiffness modified with boundary conditions\n");
    for (int j = 0; j < dof_count; ++j)
    {
        for (int i = 0; i < dof_count; ++i)
        {
            printf("%2.f ", k_boundary[i + j * dof_count]);
        }

        printf("\n");
    }

    printf("\n\n");


    // Set known boundary forces
    forces[dof_per_node * node_forced] = 1700.0f;
    forces[dof_per_node * node_forced + 1] = 0.0f;
    forces[dof_per_node * node_forced + 2] = 0.0f;

    // F = KU can now be solved for the displacements U = k^-1 * F
    // using the known boundary condition forces
    solve_jacobi(k_boundary, displacements, forces, dof_count, dof_count, 100, 1);


    // Back calculate unknown forces now that displacements are known by multipling 
    // the stiffness matrix times the displacement vector F = KU
    matrix_premultiply(forces, k_global, displacements, dof_count, dof_count);

    // Update the frames per node properties to use for rendering and analysis
    for (int i = 0; i < frame->node_count; ++i)
    {
        frame->nodes[i].force.x = forces[i * dof_per_node];
        frame->nodes[i].force.y = forces[i * dof_per_node + 1];
        frame->nodes[i].force.z = forces[i * dof_per_node + 2];

        frame->nodes[i].moment.x = forces[i * dof_per_node + 3];
        frame->nodes[i].moment.y = forces[i * dof_per_node + 4];
        frame->nodes[i].moment.z = forces[i * dof_per_node + 5];

        frame->nodes[i].displacement.x = displacements[i * dof_per_node];
        frame->nodes[i].displacement.y = displacements[i * dof_per_node + 1];
        frame->nodes[i].displacement.z = displacements[i * dof_per_node + 2];

        frame->nodes[i].rotation.y = displacements[i * dof_per_node + 3];
        frame->nodes[i].rotation.x = displacements[i * dof_per_node + 4];
        frame->nodes[i].rotation.z = displacements[i * dof_per_node + 5];
    }


    // Print Node displacments and forces

    for (int n = 0; n < frame->node_count; ++n)
    {
        printf("Node %i Displacement: (%2.2f, %2.2f, %2.2f), Rotation: (%2.2f, %2.2f, %2.2f)\n", n,
            displacements[n * dof_per_node],
            displacements[n * dof_per_node + 1],
            displacements[n * dof_per_node + 2],
            displacements[n * dof_per_node + 3],
            displacements[n * dof_per_node + 4],
            displacements[n * dof_per_node + 5]
        );
    }

    printf("\n");

    for (int n = 0; n < frame->node_count; ++n)
    {
        printf("Node %i Force: (%2.2f, %2.2f, %2.2f), Moment: (%2.2f, %2.2f, %2.2f)\n", n,
            forces[n * dof_per_node],
            forces[n * dof_per_node + 1],
            forces[n * dof_per_node + 2],
            forces[n * dof_per_node + 3],
            forces[n * dof_per_node + 4],
            forces[n * dof_per_node + 5]
        );
    }

    // Cleanup
    free(forces);
    free(displacements);
    free(k_boundary);
    free(k_global);
}

void frame_init(struct Frame* frame)
{
    // Material properties for 1040 mild steel annealed (13 C)
    frame->elastic_modulus = 200.f; // GPa
    frame->shear_modulus = 80.f; // GPa
    frame->yield_strength = 0.415f; // GPa, 415 MPa

    /* frame->elastic_modulus = 1.f; // GPa
    frame->shear_modulus = 1.f; // GPa
    frame->yield_strength = 1.f; // GPa, 415 MPa */

    const int radius = 1.0f;

    const int dof_per_node = 6;

    frame->node_count = 4;
    frame->element_count = 3;

    frame->nodes = malloc(sizeof(*frame->nodes) * frame->node_count);
    frame->elements = malloc(sizeof(*frame->elements) * frame->element_count);

    // Define node positions
    frame->nodes[0].pos = (struct vec3){ 0.f, 0.f, 0.f };
    frame->nodes[1].pos = (struct vec3){ -1.f, 0.f, 0.f };
    frame->nodes[2].pos = (struct vec3){ 0.f, -1.f, 0.f };
    frame->nodes[3].pos = (struct vec3){ 1.f, 0.f, 0.f };

    // Define an element between node1 and node2
    frame->elements[0] = (struct Element){ 0, 1, frame->elastic_modulus, frame->shear_modulus, radius };
    frame->elements[1] = (struct Element){ 0, 2, frame->elastic_modulus, frame->shear_modulus, radius };
    frame->elements[2] = (struct Element){ 0, 3, frame->elastic_modulus, frame->shear_modulus, radius };
}

void frame_release(struct Frame* frame)
{
    if (frame)
    {
        free(frame->nodes);
        free(frame->elements);

        frame->nodes = NULL;
        frame->elements = NULL;
    }
}

void frame_create_mesh(struct Frame* frame, struct Mesh* mesh)
{
    mesh->vertices_length = frame->node_count;
    mesh->vertices = malloc(sizeof(*mesh->vertices) * mesh->vertices_length);

    mesh->indices_length = 3 * frame->element_count;
    mesh->indices = malloc(sizeof(*mesh->indices) * mesh->indices_length);


    // Make a vertex for each node 
    // scaling between blue and red for x displacement
    for (int i = 0; i < frame->node_count; ++i)
    {
        float x = frame->nodes[i].displacement.x;

        float r;
        float g;
        float b;

        if (x > 0.f)
        {
            r = x;
            g = 1.f - x;
            b = 0.f;
        }
        else
        {
            r = 0.f;
            g = 1.f - x;
            b = x;
        }


        mesh->vertices[i] = (struct Vertex){
            frame->nodes[i].pos.x,
            frame->nodes[i].pos.y,
            frame->nodes[i].pos.z,
            r, g, b
        };
    }

    // Add a "triangle" for each element to draw as an edge
    for (int i = 0; i < frame->element_count; ++i)
    {
        mesh->indices[3 * i] = frame->elements[i].node1;
        mesh->indices[3 * i + 1] = frame->elements[i].node2;
        mesh->indices[3 * i + 2] = frame->elements[i].node2;
    }
}


void transform_element_stiffness(float* k_local, struct mat3 transform)
{
    // The stiffness matrix is 6x6 while the transform is only 3x3
    // Mathematically this is dealt with by composing one large transform
    // matrix by copying the transform along the diagonal

    // In this case we only need a 6x6 transform so only two copies would need to be used
    // instead we can break up the stiffness matrix into 4 quadrants transform each and compose
    // the transformed matrix from them

    // The inverse of the transformation matrix is its transpose
    struct mat3 transinv = mat3_transpose(transform);

    // Break the 6x6 stiffness into 4 3x3 quadrants
    struct mat3 quads[4];
    mat6_break_quads(k_local, quads);

    for (int i = 0; i < 4; ++i)
    {
        // post multiply each quad by transform
        struct mat3 kt = mat3_multiply(quads[i], transform);

        // Pre multiply te result by inverse transform
        struct mat3 ikt = mat3_multiply(transinv, quads[i]);
    }

    // Copy the transformed quadrants into the full local stiffness matrix
    mat6_join_quads(k_local, quads);
}

void add_element_stiffness(float* k_global, float* k_element, struct mat3 transform, int node1, int node2, int node_count)
{
    // Use the transformation matrix to transform the element stiffness matrix from local to global frame
    transform_element_stiffness(k_element, transform);

    // Add the element stiffness contribution to the global stiffness matrix
    int offset = 6 * node1 + 36 * (node2 * node_count);
    for (int j = 0; j < 6; ++j)
    {
        for (int i = 0; i < 6; ++i)
        {
            int element_idx = i + j * 6;

            int global_idx = i + offset + j * 6 * node_count;

            k_global[global_idx] += k_element[element_idx];
        }
    }
}
