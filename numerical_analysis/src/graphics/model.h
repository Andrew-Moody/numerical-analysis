#pragma once

#include <stdio.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void loadModel(const char* path)
{
    const struct aiScene* scene = aiImportFile(path, 0);

    if (!scene)
    {
        printf("Failed to load model at: %s\n", path);
    }
    else
    {
        printf("Loaded model at: %s\n", path);
    }

    aiReleaseImport(scene);
}