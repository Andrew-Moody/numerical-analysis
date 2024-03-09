#include "frameimport.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "frame.h"

int frame_import(const char* path, struct Frame* frame)
{
    // Typical file handling
    FILE* file = fopen(path, "r");

    if (!file)
    {
        fprintf(stderr, "Failed to open file at: %s\n", path);
        return -1;
    }

    // Variables to store data when reading commands
    char buffer[256];
    int count = 0;
    int check;

    // Continuously read from the file until the end or an unrecognized command
    while (1)
    {
        // Start by attempting to read a command into buffer as well as an integer count
        // if the incorrect format or end of file is encountered, break the loop
        check = fscanf(file, "%s %i", buffer, &count);
        if (check != 2)
        {
            break;
        }

        printf("%s %i\n", buffer, count);

        // Check the contents of buffer to see if it matches a valid command

        if (strcmp(buffer, "nodes") == 0)
        {
            // Allocate space to hold as many nodes as the count parameter
            frame->node_count = count;
            frame->nodes = malloc(sizeof(*frame->nodes) * frame->node_count);

            // Scan as many times as specified by count
            for (int i = 0; i < count; ++i)
            {
                int n;
                float x;
                float y;
                float z;

                // Read a line for each node to get the node index and position vector
                if (fscanf(file, "%i %f %f %f", &n, &x, &y, &z) != 4)
                {
                    fprintf(stderr, "Frame Import Error: Invalid node format\n");
                    break;
                }

                printf("%i, %f, %f, %f\n", n, x, y, z);

                // Initialize a node with the values that were read
                frame->nodes[i] = (struct Node){ (struct vec3) { x, y, z } };
            }
        }
        else if (strcmp(buffer, "elements") == 0)
        {
            // Allocate space to hold as many elements as the count parameter
            frame->element_count = count;
            frame->elements = malloc(sizeof(*frame->elements) * frame->element_count);

            // Scan as many times as specified by count
            for (int i = 0; i < count; ++i)
            {
                int n1;
                int n2;
                float elastic_modulus;
                float shear_modulus;
                float radius;

                // Read a line for each node to get the stat and end node indices and material properties
                if (fscanf(file, "%i %i %f %f %f", &n1, &n2, &elastic_modulus, &shear_modulus, &radius) != 5)
                {
                    fprintf(stderr, "Frame Import Error : Invalid element format for element %i\n", i);
                    break;
                }

                printf("%i, %i, %f, %f, %f\n", n1, n2, elastic_modulus, shear_modulus, radius);

                // Initialize an element with the values that were read
                frame->elements[i] = (struct Element){ n1, n2, elastic_modulus, shear_modulus, radius };
            }
        }
        else if (strcmp(buffer, "boundary_conditions") == 0)
        {
            // Allocate space to hold as many boundary conditions as the count parameter
            frame->bc_count = count;
            frame->bconditions = malloc(sizeof(*frame->bconditions) * frame->bc_count);

            // Scan as many times as specified by count
            for (int i = 0; i < count; ++i)
            {
                int n;
                float x;
                float y;
                float z;

                // Read a line for each boundary condition to get the node index it applies to
                // a vector value and a string corresponding to the property type
                if (fscanf(file, "%i %f %f %f %s", &n, &x, &y, &z, buffer) != 5)
                {
                    fprintf(stderr, "Frame Import Error: Invalid boundary condition format\n");
                    break;
                }

                // Determine the boundary condition kind by comparing the string in the buffer
                enum BoundaryKind boundary_kind;

                if (strcmp(buffer, "force") == 0)
                {
                    boundary_kind = BC_Force;
                }
                else if (strcmp(buffer, "displacement") == 0)
                {
                    boundary_kind = BC_Displacement;
                }
                else if (strcmp(buffer, "moment") == 0)
                {
                    boundary_kind = BC_Moment;
                }
                else if (strcmp(buffer, "rotation") == 0)
                {
                    boundary_kind = BC_Rotation;
                }
                else if (strcmp(buffer, "joint") == 0)
                {
                    boundary_kind = BC_Joint;
                }
                else
                {
                    fprintf(stderr, "Frame Import Error: unknown boundary condition type: %s\n", buffer);
                    break;
                }

                // Initialize a boundary condition with the values that were read
                frame->bconditions[i] = (struct BoundaryCondition){ n, boundary_kind, (struct vec3) { x, y, z } };

                //printf("%i, %f, %f, %f, %s\n", n, x, y, z, buffer);

                printf("%i, %f, %f, %f, %s, %i\n", n,
                    frame->bconditions[i].value.x,
                    frame->bconditions[i].value.y,
                    frame->bconditions[i].value.z,
                    buffer,
                    boundary_kind
                );
            }
        }
        else
        {
            fprintf(stderr, "Frame Import Error: unrecognized command: %s\n", buffer);
            break;
        }
    }


    // Close the file before returning regardless of whether the file was parsed correctly
    fclose(file);

    // Return success if the end of the file was reached without incident
    if (check != EOF)
    {
        fprintf(stderr, "Frame Import Error: Invalid command format\n");
        return -1;
    }

    printf("End of File\n\n");
    return 0;
}


void frame_create_sample(struct Frame* frame)
{
    // Material properties for 1040 mild steel annealed (13 C)
    const float elastic_modulus = 200.f; // GPa
    const float shear_modulus = 80.f; // GPa
    const float yield_strength = 0.415f; // GPa, 415 MPa

    const int radius = 1.0f;

    // Four nodes
    frame->node_count = 4;
    frame->element_count = 3;
    frame->bc_count = 4;

    frame->nodes = malloc(sizeof(*frame->nodes) * frame->node_count);
    frame->elements = malloc(sizeof(*frame->elements) * frame->element_count);
    frame->bconditions = malloc(sizeof(*frame->bconditions) * frame->bc_count);

    // Define node positions
    frame->nodes[0].pos = (struct vec3){ 0.f, 0.f, 0.f };
    frame->nodes[1].pos = (struct vec3){ -1.f, 0.f, 0.f };
    frame->nodes[2].pos = (struct vec3){ 0.f, -1.f, 0.f };
    frame->nodes[3].pos = (struct vec3){ 1.f, 0.f, 0.f };

    // Define an elements
    frame->elements[0] = (struct Element){ 0, 1, elastic_modulus, shear_modulus, radius };
    frame->elements[1] = (struct Element){ 0, 2, elastic_modulus, shear_modulus, radius };
    frame->elements[2] = (struct Element){ 0, 3, elastic_modulus, shear_modulus, radius };

    // Define boundary conditions

    // Fixed position at nodes 1, 2, and 3 (but not rotation)
    frame->bconditions[1].node = 1;
    frame->bconditions[1].kind = BC_Displacement;
    frame->bconditions[1].value = (struct vec3){ 0.f, 0.f, 0.f };

    frame->bconditions[2].node = 2;
    frame->bconditions[2].kind = BC_Displacement;
    frame->bconditions[2].value = (struct vec3){ 0.f, 0.f, 0.f };

    frame->bconditions[3].node = 3;
    frame->bconditions[3].kind = BC_Displacement;
    frame->bconditions[3].value = (struct vec3){ 0.f, 0.f, 0.f };

    // Applied force at node 0
    frame->bconditions[0].node = 0;
    frame->bconditions[0].kind = BC_Force;
    frame->bconditions[0].value = (struct vec3){ 700.f, 0.f, 0.f };
}
