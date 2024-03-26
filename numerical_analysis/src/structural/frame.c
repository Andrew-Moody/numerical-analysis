#include "frame.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "vector.h"
#include "matrix.h"
#include "linearsolve.h"
#include "mesh.h"

#include "frameprocess.h"

// Degrees of freedom (3 translation and 3 rotation)
#define DOF 6

// Forward Declarations
void build_stiffness(struct Frame* frame, struct Matrix* k_global);
void transform_element_stiffness(struct mat6* k_local, struct mat3 transform);
void add_element_stiffness(struct Matrix* k_global, struct mat6* k_element, struct mat3 transform, int node1, int node2, int node_count);
void apply_boundary_conditions(struct Frame* frame, struct Matrix* stiffness, struct vecf* forces);

void frame_build_equations(struct Frame* frame, struct EquationSet* eqset)
{
    int dof_count = DOF * frame->node_count;

    build_stiffness(frame, &eqset->stiffness);

    // Alocate and fill vectors for force and displacement
    vecf_init(&eqset->forces, dof_count);
    vecf_fill(&eqset->forces, 0.0f);
    vecf_init(&eqset->displacements, dof_count);
    vecf_fill(&eqset->displacements, 0.0f);

    // Copy stiffness before appling boundary conditions
    matrix_copy(&eqset->stiff_bc, &eqset->stiffness);

    // Modify the copied stiffness matrix to reflect the boundary conditions
    apply_boundary_conditions(frame, &eqset->stiff_bc, &eqset->forces);
}

void build_stiffness(struct Frame* frame, struct Matrix* k_global)
{
    int dof_count = DOF * frame->node_count;

    matrix_init(k_global, dof_count, dof_count, 1);

    // Add each elements contribution to the stiffness matrix
    for (int element_idx = 0; element_idx < frame->element_count; ++element_idx)
    {
        const int node1 = frame->elements[element_idx].node1;
        const int node2 = frame->elements[element_idx].node2;

        const float length = vec3_distance(frame->nodes[node1].pos, frame->nodes[node2].pos);
        const float l2 = length * length;
        const float l3 = l2 * length;

        // Convert Gigapascals to Pascals
        const float e = frame->elements[element_idx].elastic_modulus * 1000000000;
        const float g = frame->elements[element_idx].shear_modulus * 1000000000;
        const float r = frame->elements[element_idx].radius;

        //printf("Elastic: %f, Shear: %f, Radius %f, Length %f\n", e, g, r, length);

        // Cross sectional area
        const float area = r * r * 3.14159f;

        // Area moment of inertia about y and z axes(for bending) m^4
        //const double inertia_y = r * r * r * r * 3.14159f / 4.f;
        //const double inertia_z = inertia_y;

        const float inertia_y = r * r * r * r * 3.14159f / 4.f;
        const float inertia_z = inertia_y;

        // Polar moment of inertia about the x axis (for torsion) m^4
        const float inertia_x = r * r * r * r * 3.14159f / 2.f;
        //const double inertia_x = r * r * r * r * 3.14159f * 5.f / 2.f;

        // Pre multipling common factors
        const float ei_y = e * inertia_y;
        const float ei_z = e * inertia_z;
        const float gj = g * inertia_x;

        /* printf("Ix: %.12f, Iy: %.12f, Iz: %.12f\n", inertia_x, inertia_y, inertia_z);

        printf("Ix: %.12f, Iy: %.12f, Iz: %.12f\n", gj, ei_y, ei_z); */

        // Axial Stiffness (for tension/compression along axis)
        const float k = e * area / length;

        // At each node there are 3 forces and 3 moments that produce 3 displacements and 3 rotations
        // we can build a 6x6 matrix that represents the relationship between the forces and moments
        // at one node and the displacements and rotations at another
        // each element will produce 4 of these 6x6 matrices one for each combination
        // they are mostly similar but some components vary in sign
        // These 6x6 matrices are then slotted in to the global matrix after transforming to global frame
        // The are pretty simple for axial stiffness only

        // kxy is the stiffness matrix for force and moment on node x due to the displacement and rotation of node y

        // General stiffness matrices for beams (fixed joints) that include
        // bending about y and z and torsion about x along with axial stiffness
        struct mat6 k11[36] = {
            k, 0, 0, 0, 0, 0,
            0, (12 * ei_z / l3), 0, 0, 0, (6 * ei_z / l2),
            0, 0, (12 * ei_y / l3), 0, -(6 * ei_y / l2), 0,
            0, 0, 0, (gj / length), 0, 0,
            0, 0, -(6 * ei_y / l2), 0, (4 * ei_y / length), 0,
            0, (6 * ei_z / l2), 0, 0, 0, (4 * ei_z / length)
        };

        struct mat6 k12[36] = {
            -k, 0, 0, 0, 0, 0,
            0, -(12 * ei_z / l3), 0, 0, 0, (6 * ei_z / l2),
            0, 0, -(12 * ei_y / l3), 0, -(6 * ei_y / l2), 0,
            0, 0, 0, -(gj / length), 0, 0,
            0, 0, (6 * ei_y / l2), 0, (2 * ei_y / length), 0,
            0, -(6 * ei_z / l2), 0, 0, 0, (2 * ei_z / length)
        };

        struct mat6 k21[36] = {
            -k, 0, 0, 0, 0, 0,
            0, -(12 * ei_z / l3), 0, 0, 0, -(6 * ei_z / l2),
            0, 0, -(12 * ei_y / l3), 0, (6 * ei_y / l2), 0,
            0, 0, 0, -(gj / length), 0, 0,
            0, 0, -(6 * ei_y / l2), 0, (2 * ei_y / length), 0,
            0, (6 * ei_z / l2), 0, 0, 0, (2 * ei_z / length)
        };

        struct mat6 k22[36] = {
            k, 0, 0, 0, 0, 0,
            0, (12 * ei_z / l3), 0, 0, 0, -(6 * ei_z / l2),
            0, 0, (12 * ei_y / l3), 0, (6 * ei_y / l2), 0,
            0, 0, 0, (gj / length), 0, 0,
            0, 0, (6 * ei_y / l2), 0, (4 * ei_y / length), 0,
            0, -(6 * ei_z / l2), 0, 0, 0, (4 * ei_z / length)
        };

        // Axial stiffness only for trusses (pinned-joints that are free to rotate)
        /* struct mat6 k11[36] = {
            k, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
        };

        struct mat6 k12[36] = {
            -k, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
        };

        struct mat6 k21[36] = {
            -k, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
        };

        struct mat6 k22[36] = {
            k, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
        }; */


        /* printf("k11:\n");
        matrix_print(k11, 6, 6);

        printf("k12:\n");
        matrix_print(k12, 6, 6);

        printf("k21:\n");
        matrix_print(k21, 6, 6);

        printf("k22:\n");
        matrix_print(k22, 6, 6); */

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


        /* printf("x_axis: %2.f, %2.f, %2.f\n", x_axis.x, x_axis.y, x_axis.z);
        printf("y_axis: %2.f, %2.f, %2.f\n", y_axis.x, y_axis.y, y_axis.z);
        printf("z_axis: %2.f, %2.f, %2.f\n\n", z_axis.x, z_axis.y, z_axis.z); */

        // Add the local contributions to the global stiffness matrix
        add_element_stiffness(k_global, k11, transform, node1, node1, frame->node_count);
        add_element_stiffness(k_global, k12, transform, node1, node2, frame->node_count);
        add_element_stiffness(k_global, k21, transform, node2, node1, frame->node_count);
        add_element_stiffness(k_global, k22, transform, node2, node2, frame->node_count);
    }


    //printf("Global Stiffness Matrix\n");
    //matrix_print(k_global, dof_count, dof_count);
}

void frame_update_results(struct Frame* frame, struct EquationSet* eqset)
{
    int dof_count = DOF * frame->node_count;

    struct Matrix* stiffness = &eqset->stiffness;
    struct Matrix* stiff_bc = &eqset->stiff_bc;
    struct vecf* forces = &eqset->forces;
    struct vecf* displacements = &eqset->displacements;

    // Back calculate unknown forces now that displacements are known by multipling 
    // the full (not boundary) stiffness matrix times the displacement vector F = KU
    matrix_premultiply(forces->elements, stiffness->elements, displacements->elements, dof_count, dof_count);
    //matrix_premultiply(forces, stiffness, displacements);

    //printf("Solved Forces:\n");
    //matrix_print(forces, dof_count, 1);

    // Update the frames per node properties to use for rendering and analysis
    for (int i = 0; i < frame->node_count; ++i)
    {
        frame->nodes[i].force.x = forces->elements[i * DOF];
        frame->nodes[i].force.y = forces->elements[i * DOF + 1];
        frame->nodes[i].force.z = forces->elements[i * DOF + 2];

        frame->nodes[i].moment.x = forces->elements[i * DOF + 3];
        frame->nodes[i].moment.y = forces->elements[i * DOF + 4];
        frame->nodes[i].moment.z = forces->elements[i * DOF + 5];

        frame->nodes[i].displacement.x = displacements->elements[i * DOF];
        frame->nodes[i].displacement.y = displacements->elements[i * DOF + 1];
        frame->nodes[i].displacement.z = displacements->elements[i * DOF + 2];

        frame->nodes[i].rotation.y = displacements->elements[i * DOF + 3];
        frame->nodes[i].rotation.x = displacements->elements[i * DOF + 4];
        frame->nodes[i].rotation.z = displacements->elements[i * DOF + 5];
    }
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
    for (int i = 0; i < frame->node_count; ++i)
    {
        // Default color to grey
        mesh->vertices[i] = (struct Vertex){
            frame->nodes[i].pos.x,
            frame->nodes[i].pos.y,
            frame->nodes[i].pos.z,
            0.6f, 0.6f, 0.6f
            //0.3f, 0.3f, 0.3f
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


void frame_mesh_recolor(const struct Frame* frame, struct Mesh* mesh, color_func_t color_func)
{
    if (frame->node_count != mesh->vertices_length)
    {
        fprintf(stderr, "Error Recoloring mesh: frame node count does not match mesh vertex count");
        return;
    }

    for (int i = 0; i < mesh->vertices_length; ++i)
    {
        color_func(&mesh->vertices[i], &frame->nodes[i]);
    }
}


void transform_element_stiffness(struct mat6* k_local, struct mat3 transform)
{
    // The stiffness matrix is 6x6 while the transform is only 3x3
    // Mathematically this is dealt with by composing one large transform
    // matrix by copying the transform along the diagonal

    // In this case we only need a 6x6 transform so only two copies would need to be used
    // instead we can break up the stiffness matrix into 4 quadrants transform each and compose
    // the transformed matrix from them

    // The inverse of the transformation matrix is its transpose
    struct mat3 transinv = mat3_transpose(transform);

    /* printf("Transform:\n");
    matrix_print(transform.elements, 3, 3);

    printf("Inverse Transform:\n");
    matrix_print(transinv.elements, 3, 3); */

    // Break the 6x6 stiffness into 4 3x3 quadrants
    struct mat3 quads[4];
    mat6_break_quads(k_local->elements, quads);

    for (int i = 0; i < 4; ++i)
    {
        // post multiply each quad by the transform
        quads[i] = mat3_multiply(quads[i], transform);

        // Pre multiply the result by inverse transform
        quads[i] = mat3_multiply(transinv, quads[i]);
    }

    // Copy the transformed quadrants into the full local stiffness matrix
    mat6_join_quads(k_local->elements, quads);
}

void add_element_stiffness(struct Matrix* k_global, struct mat6* k_element, struct mat3 transform, int node1, int node2, int node_count)
{
    //printf("Element stiffness for nodes %i and %i:\nLocal:\n", node1, node2);
    //mat6_print(*k_element);

    // Use the transformation matrix to transform the element stiffness matrix from local to global frame
    transform_element_stiffness(k_element, transform);

    //printf("Global:\n");
    //mat6_print(*k_element);

    // Add the element stiffness contribution to the global stiffness matrix
    int offset = 6 * node2 + 36 * (node1 * node_count);

    // Incorrect results in k12 and k21 being swapped (3 hours later)
    //int offset = 6 * node1 + 36 * (node2 * node_count); 

    for (int j = 0; j < 6; ++j)
    {
        for (int i = 0; i < 6; ++i)
        {
            int element_idx = i + j * 6;

            int global_idx = i + offset + j * 6 * node_count;

            k_global->elements[global_idx] += k_element->elements[element_idx];
        }
    }
}


void apply_displacement(int dof, float value, struct Matrix* stiffness, int length)
{
    // Set the global stiffness rows and columns for the affected degree of freedom
    // to 1 on diagonal and 0 everywhere else (setting diagonals to 1 is not strictly
    // necessary but facilitates some solution methods)
    // other options include building a new matrix with reduced degrees of freedom
    // which might lead to faster solution if the there are many boundary conditions
    int rowstart = dof * length;
    int colstart = dof;

    for (int i = 0; i < length; ++i)
    {
        // Set row to zero
        stiffness->elements[rowstart + i] = 0;

        //Set column to zero
        stiffness->elements[colstart + i * length] = 0;
    }

    // Set diagonal to 1
    stiffness->elements[rowstart + colstart] = 1;
}

void apply_boundary_conditions(struct Frame* frame, struct Matrix* stiffness, struct vecf* forces)
{
    int length = DOF * frame->node_count;

    // Modify the stiffness matrix and force vector to reflect the boundary conditions on the frame
    for (int n = 0; n < frame->bc_count; ++n)
    {
        int node = frame->bconditions[n].node;
        struct vec3 value = frame->bconditions[n].value;

        if (frame->bconditions[n].kind == BC_Displacement)
        {
            if (value.x != 0.f || value.y != 0.f || value.z != 0.f)
            {
                printf("Warning: Non-homogeneous boundary conditions applied: (%f, %f, %f\n", value.x, value.y, value.z);
                return;
            }

            apply_displacement(node * 6, value.x, stiffness, length);
            apply_displacement(node * 6 + 1, value.y, stiffness, length);
            apply_displacement(node * 6 + 2, value.z, stiffness, length);
        }
        else if (frame->bconditions[n].kind == BC_Rotation)
        {
            if (value.x != 0.f || value.y != 0.f || value.z != 0.f)
            {
                printf("Warning: Non-homogeneous boundary conditions applied: (%f, %f, %f\n", value.x, value.y, value.z);
                return;
            }

            apply_displacement(node * 6 + 3, value.x, stiffness, length);
            apply_displacement(node * 6 + 4, value.y, stiffness, length);
            apply_displacement(node * 6 + 5, value.z, stiffness, length);
        }
        else if (frame->bconditions[n].kind == BC_Force)
        {
            // Set known boundary forces
            forces->elements[DOF * node] = frame->bconditions[n].value.x;
            forces->elements[DOF * node + 1] = frame->bconditions[n].value.y;
            forces->elements[DOF * node + 2] = frame->bconditions[n].value.z;
        }
        else if (frame->bconditions[n].kind == BC_Moment)
        {
            // Set known boundary moments
            forces->elements[DOF * node + 3] = frame->bconditions[n].value.x;
            forces->elements[DOF * node + 4] = frame->bconditions[n].value.y;
            forces->elements[DOF * node + 5] = frame->bconditions[n].value.z;
        }
        else if (frame->bconditions[n].kind == BC_Joint)
        {
            // Little bit different since instead of float magnitudes
            // we are using the value as type code

            float joints[3];
            joints[0] = frame->bconditions[n].value.x;
            joints[1] = frame->bconditions[n].value.y;
            joints[2] = frame->bconditions[n].value.z;

            // its a little bit difficult to handle joints with elements sharing nodes
            // if you want the joint for one element to be fixed and the other hinged
            // for instance if you have a frame made of two fixed elements with a third 
            // element attached with a hinge

            // This will treat all joints at a node the same so if you want different behavior
            // the best way would be to use an additional node coincident with the joint
            // with an infinitely stiff element between the two (may have to check for zero length)
            // then have a fixed joint at the original node and a hinged joint at the other

            for (int i = 0; i < 3; ++i)
            {
                switch ((int)joints[i])
                {
                case 0:
                {
                    // Fixed Joint (no rotation)

                    break;
                }
                case 1:
                {
                    // Hinged joint (pinned)
                    // Need to fix the moment in the pinned axis to zero
                    // and cancel out the related stiffness matrix terms
                    // otherwise the zero moment will get overwritten
                }
                default:
                {
                    fprintf(stderr, "Warning: unhandled boundary condition joint type (%i, %f) for node: %i",
                        (int)joints[i], joints[i], frame->bconditions[n].node
                    );
                }
                }
            }
        }
    }
}

void equationset_release(struct EquationSet* eqset)
{
    if (eqset)
    {
        vecf_release(&eqset->forces);
        vecf_release(&eqset->displacements);
        matrix_release(&eqset->stiffness);
        matrix_release(&eqset->stiff_bc);
    }
}

void frame_print_results(struct Frame* frame)
{
    // Need to work on formatting
    printf("          Displacement              Rotation              Force              Moment\n");

    for (int n = 0; n < frame->node_count; ++n)
    {
        struct Node* node = &frame->nodes[n];

        printf("Node %i | (%2.3f, %2.3f, %2.3f) | (%2.3f, %2.3f, %2.3f) | "
            "(% 2.3f, % 2.3f, % 2.3f) | (% 2.3f, % 2.3f, % 2.3f)\n\n", n,
            node->displacement.x,
            node->displacement.y,
            node->displacement.z,

            node->rotation.x,
            node->rotation.y,
            node->rotation.z,

            node->force.x,
            node->force.y,
            node->force.z,

            node->moment.x,
            node->moment.y,
            node->moment.z
        );
    }
}

void equationset_print(struct EquationSet* eqset)
{
    printf("\n");
    matrix_print(eqset->stiff_bc);
    printf("\n");


    for (int n = 0; n < eqset->displacements.count / 6; ++n)
    {
        printf("Node %i Displacement: (%2.3f, %2.3f, %2.3f), Rotation: (%2.3f, %2.3f, %2.3f), "
            "Force: (% 2.3f, % 2.3f, % 2.3f), Moment: (% 2.3f, % 2.3f, % 2.3f)\n", n,
            eqset->displacements.elements[n * DOF],
            eqset->displacements.elements[n * DOF + 1],
            eqset->displacements.elements[n * DOF + 2],
            eqset->displacements.elements[n * DOF + 3],
            eqset->displacements.elements[n * DOF + 4],
            eqset->displacements.elements[n * DOF + 5],
            eqset->forces.elements[n * DOF],
            eqset->forces.elements[n * DOF + 1],
            eqset->forces.elements[n * DOF + 2],
            eqset->forces.elements[n * DOF + 3],
            eqset->forces.elements[n * DOF + 4],
            eqset->forces.elements[n * DOF + 5]
        );
    }
}

void frame_solve(struct Frame* frame)
{
    struct EquationSet eqset;

    frame_build_equations(frame, &eqset);

    // F = KU can now be solved for the displacements U = k^-1 * F
    // using the known boundary condition forces
    int iterations = 100;
    struct vecf residuals;
    vecf_init(&residuals, iterations);
    solve_jacobi_single(eqset, residuals.elements, iterations);

    // Populate per node properties using displacements to back calculate forces
    frame_update_results(frame, &eqset);

    printf("\n");
    equationset_print(&eqset);
    printf("\n");

    // Print Node displacments and forces
    frame_print_results(frame);

    // Cleanup
    vecf_release(&residuals);
    equationset_release(&eqset);
}

void color_disp_x(struct Vertex* vertex, struct Node* node)
{
    float x = node->displacement.x;

    vertex->r = fmaxf(x, 0.f);
    vertex->g = 1.f - fabsf(x);
    vertex->b = fmaxf(-x, 0.f);
}

void color_disp_y(struct Vertex* vertex, struct Node* node)
{
    float y = node->displacement.y;

    vertex->r = fmaxf(y, 0.f);
    vertex->g = 1.f - fabsf(y);
    vertex->b = fmaxf(-y, 0.f);
}

void color_parallel_multicolor(struct Vertex* vertex, struct Node* node)
{
    switch (node->multicolor)
    {
    case 0:
        vertex->r = 0.0f;
        vertex->g = 0.0f;
        vertex->b = 0.0f;
        break;
    case 1:
        vertex->r = 1.0f;
        vertex->g = 1.0f;
        vertex->b = 0.0f;
        break;
    case 2:
        vertex->r = 1.0f;
        vertex->g = 0.0f;
        vertex->b = 1.0f;
        break;
    case 3:
        vertex->r = 0.0f;
        vertex->g = 1.0f;
        vertex->b = 1.0f;
        break;
    case 4:
        vertex->r = 0.0f;
        vertex->g = 0.1f;
        vertex->b = 0.5f;
        break;
    case 5:
        vertex->r = 1.0f;
        vertex->g = 0.1f;
        vertex->b = 0.5f;
        break;
    case 6:
        vertex->r = 0.0f;
        vertex->g = 0.0f;
        vertex->b = 1.0f;
        break;
    case 7:
        vertex->r = 0.0f;
        vertex->g = 1.0f;
        vertex->b = 0.0f;
        break;
    case 8:
        vertex->r = 1.0f;
        vertex->g = 0.0f;
        vertex->b = 0.0f;
        break;
    default:
        vertex->r = 0.0f;
        vertex->g = 0.0f;
        vertex->b = 0.0f;
        break;
    }
}