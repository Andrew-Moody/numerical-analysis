#pragma once

#include <stdlib.h>

#include "mesh.h"

int create_grid_mesh(int x_steps, int y_steps, struct Mesh* mesh)
{
    mesh->vertices_length = (x_steps + 1) * (y_steps + 1);

    mesh->vertices = (struct Vertex*)malloc(mesh->vertices_length * sizeof(struct Vertex));

    if (!mesh->vertices)
    {
        return -1;
    }

    // Set vertices
    for (int j = 0; j <= y_steps; ++j)
    {
        for (int i = 0; i <= x_steps; ++i)
        {
            int index = i + j * (x_steps + 1);

            mesh->vertices[index].x = -0.5f + (float)i / x_steps;
            mesh->vertices[index].y = -0.5f + (float)j / y_steps;
            mesh->vertices[index].z = 0.0f;

            mesh->vertices[index].r = (float)i / x_steps;
            mesh->vertices[index].g = (float)j / y_steps;
            mesh->vertices[index].b = 0.0f;
        }
    }

    mesh->indices_length = 6 * x_steps * y_steps;

    mesh->indices = (unsigned int*)malloc(mesh->indices_length * sizeof(unsigned int));

    if (!mesh->indices)
    {
        return -1;
    }

    // Set indices
    for (int j = 0; j < y_steps; ++j)
    {
        for (int i = 0; i < x_steps; ++i)
        {
            int triangle_idx = 6 * (i + j * x_steps);

            int v1 = (i + j * (y_steps + 1));
            int v2 = v1 + 1;
            int v3 = (i + (j + 1) * (y_steps + 1));
            int v4 = v3 + 1;

            // Note winding order
            mesh->indices[triangle_idx] = v1;
            mesh->indices[triangle_idx + 1] = v2;
            mesh->indices[triangle_idx + 2] = v3;

            mesh->indices[triangle_idx + 3] = v2;
            mesh->indices[triangle_idx + 4] = v4;
            mesh->indices[triangle_idx + 5] = v3;
        }
    }
}