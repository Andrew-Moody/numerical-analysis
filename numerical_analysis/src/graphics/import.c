#include "import.h"

#include <stdlib.h>
#include <stdio.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/mesh.h>

#include "mesh.h"

// Convert imported mesh data into a Mesh instance
void processMesh(const struct aiScene* scene, const struct aiMesh* ai_mesh, struct Mesh* mesh)
{
    // the number of vertices (and other per vertex properties) for the current mesh
    unsigned int vert_count = ai_mesh->mNumVertices;

    mesh->vertices_length = vert_count;
    mesh->vertices = (struct Vertex*)malloc(sizeof(struct Vertex) * mesh->vertices_length);

    if (!mesh->vertices)
    {
        return;
    }

    for (int i = 0; i < vert_count; ++i)
    {
        mesh->vertices[i].x = ai_mesh->mVertices[i].x;
        mesh->vertices[i].y = ai_mesh->mVertices[i].y;
        mesh->vertices[i].z = ai_mesh->mVertices[i].z;

        mesh->vertices[i].r = ai_mesh->mVertices[i].x * 0.025f;
        mesh->vertices[i].g = ai_mesh->mVertices[i].z * 0.025f;
        mesh->vertices[i].b = -ai_mesh->mVertices[i].y * 0.025f;
    }

    unsigned int face_count = ai_mesh->mNumFaces;

    mesh->indices_length = 3 * face_count;
    mesh->indices = (unsigned int*)malloc(sizeof(unsigned int) * mesh->indices_length);

    if (!mesh->indices)
    {
        return;
    }

    for (int i = 0; i < face_count; ++i)
    {
        if (ai_mesh->mFaces[i].mNumIndices != 3)
        {
            printf("Face has num indices != 3\n");
        }
        mesh->indices[3 * i] = ai_mesh->mFaces[i].mIndices[0];
        mesh->indices[3 * i + 1] = ai_mesh->mFaces[i].mIndices[1];
        mesh->indices[3 * i + 2] = ai_mesh->mFaces[i].mIndices[2];
    }
}

// Traverse the scene heirarchy looking for a mesh to process
void processNode(const struct aiScene* scene, const struct aiNode* node, struct Mesh* mesh)
{
    const char* name = node->mName.data;

    printf("Processing node named %s, with %i meshes and %i child nodes\n", node->mName.data, node->mNumMeshes, node->mNumChildren);

    // Process each mesh in the current node
    for (int i = 0; i < node->mNumMeshes; ++i)
    {
        // all meshes are held in a single array owned by the scene
        // node->mMeshes contains the indices for the meshes that belong to this node
        unsigned int mesh_idx = node->mMeshes[i];

        processMesh(scene, scene->mMeshes[mesh_idx], mesh);
    }

    // Continue processing child nodes
    for (int i = 0; i < node->mNumChildren; ++i)
    {
        processNode(scene, node->mChildren[i], mesh);
    }
}

void load_stl(const char* path, struct Mesh* mesh)
{
    if (!path)
    {
        return;
    }

    printf("Attempting to load file at: %s\n", path);

    // May not be necessary for STL files since they triangles should be the only primitive, but some model formats
    // include points, lines, and polygons with more than 3 vertices
    // aiProcess_Triangulate converts polygons to triangles (does not apply to point and line primitives)
    // aiProcess_SortByPType splits meshes with multiple primitive types (after triangulation) into submeshes
    // that each contain a single primitive. Typically point and line meshes can then be ignored
    // setting #define AI_CONFIG_PP_SBP_REMOVE aiPrimitiveType_POINT | aiPrimitiveType_LINE
    // along with aiProcess_SortByPType causes points and lines to be excluded from the scene entirely

    unsigned int post_process_flags = 0;

    /*
    // Convert higher order polygons to triangles and discard point and line primitives
    #define AI_CONFIG_PP_SBP_REMOVE aiPrimitiveType_POINT | aiPrimitiveType_LINE
        post_process_flags = post_process_flags | aiProcess_Triangulate | aiProcess_SortByPType;
    */

    // post_process_flags = post_process_flags | aiProcess_FlipUVs; // Textures may need to be reversed for OpenGL

    const struct aiScene* scene = aiImportFile(path, post_process_flags);

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
    {
        const char* error = aiGetErrorString();

        if (error)
        {
            fprintf(stderr, "Failed to load model at: %s\nAssimp Error: %s", path, aiGetErrorString());
        }
        else
        {
            fprintf(stderr, "Failed to load model at: %s\n", path);
        }

        aiReleaseImport(scene); // Resources may still need to be released in case of Incomplete flag
        return;
    }

    processNode(scene, scene->mRootNode, mesh);

    // Don't have the RAII benefits of the Importer class in C so must release scene resources manually
    // Do not call free (or delete) on scene or any of its contents
    aiReleaseImport(scene);
}
