#include "frameimport.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "frame.h"

int frame_import(const char* path, struct Frame* frame)
{
    FILE* file = fopen(path, "r");

    if (!file)
    {
        fprintf(stderr, "Failed to open file at: %s", path);
        return -1;
    }

    char buffer[256];
    int count = 0;

    while (1)
    {
        if (fscanf(file, "%s %i", buffer, &count) < 2)
        {
            break;
        }

        printf("%s %i\n", buffer, count);

        if (strcmp(buffer, "nodes") == 0)
        {
            frame->node_count = count;
            frame->nodes = malloc(sizeof(*frame->nodes) * frame->node_count);

            for (int i = 0; i < count; ++i)
            {
                int n;
                float x;
                float y;
                float z;

                if (fscanf(file, "%i %f %f %f", &n, &x, &y, &z) != 4)
                {
                    fprintf(stderr, "Error parsing file at: %s", path);
                    return -1;
                }

                printf("%i, %f, %f, %f\n", n, x, y, z);

                frame->nodes[i] = (struct Node){ (struct vec3) { x, y, z } };
            }
        }
        else if (strcmp(buffer, "elements") == 0)
        {
            frame->element_count = count;
            frame->elements = malloc(sizeof(*frame->elements) * frame->element_count);

            for (int i = 0; i < count; ++i)
            {
                int n1;
                int n2;
                float elastic_modulus;
                float shear_modulus;
                float radius;

                if (fscanf(file, "%i %i %f %f %f", &n1, &n2, &elastic_modulus, &shear_modulus, &radius) != 5)
                {
                    fprintf(stderr, "Error parsing file at: %s", path);
                    return -1;
                }

                printf("%i, %i, %f, %f, %f\n", n1, n2, elastic_modulus, shear_modulus, radius);

                frame->elements[i] = (struct Element){ n1, n2, elastic_modulus, shear_modulus, radius };
            }
        }
        else if (strcmp(buffer, "boundary_conditions") == 0)
        {
            frame->bc_count = count;
            frame->bconditions = malloc(sizeof(*frame->bconditions) * frame->bc_count);

            for (int i = 0; i < count; ++i)
            {
                int n;
                float x;
                float y;
                float z;

                if (fscanf(file, "%i %f %f %f %s", &n, &x, &y, &z, buffer) != 5)
                {
                    fprintf(stderr, "Error parsing file at: %s", path);
                    return -1;
                }

                enum BoundaryKind boundary_kind;

                if (strcmp(buffer, "force") == 0)
                {
                    boundary_kind = BC_Force;
                }
                else if (strcmp(buffer, "displacement") == 0)
                {
                    boundary_kind = BC_Displacement;
                }
                else
                {
                    fprintf(stderr, "Error parsing file at: %s", path);
                    return -1;
                }

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
    }

    printf("End of File\n");

    fclose(file);

    return 0;
}