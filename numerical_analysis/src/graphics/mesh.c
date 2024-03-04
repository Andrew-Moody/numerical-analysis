#include "mesh.h"

#include <stdlib.h>

void mesh_release(struct Mesh* mesh)
{
    if (!mesh)
    {
        return;
    }

    free(mesh->vertices);
    free(mesh->indices);
}