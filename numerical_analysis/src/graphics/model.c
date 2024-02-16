#include "model.h"

#include <stdlib.h>
#include <stdio.h>

// glad must be included before glfw
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "mesh.h"

void model_init(struct Model* model, struct Mesh* mesh)
{
    if (!model || !mesh)
    {
        return;
    }

    model->mesh = mesh;

    // OpenGL resources are identified by id number

    // Generate and bind a Vertex Array Object (to store attributes)
    // While bound, Vertex Array Object stores information about the bound Vertex Buffer Object
    // as calls are made to modify properties on the VBO
    glGenVertexArrays(1, &model->vertex_array_obj);
    glBindVertexArray(model->vertex_array_obj);

    // Generate and bind a vertex buffer object (to store actual vertex data)
    // any calls targeting GL_ARRAY_BUFFER will effect this vbo until it is unbound
    glGenBuffers(1, &model->vertex_buffer_obj);
    glBindBuffer(GL_ARRAY_BUFFER, model->vertex_buffer_obj);

    // Copy vertex data to gpu memory
    glBufferData(GL_ARRAY_BUFFER, mesh->vertices_length * sizeof(float), mesh->vertices, GL_STATIC_DRAW);

    // Tell the gpu how the vertex buffer data is laid out (vbo is still bound) this can be done manually
    // each time the vbo is bound or use the vao to store this info and use it when the vao is bound
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    if (mesh->indices)
    {
        // Generate and bind an element buffer object (to store the list of indices that describe the triangles to draw)
        glGenBuffers(1, &model->element_buffer_obj);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->element_buffer_obj);

        // Copy index data to gpu memory
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices_length * sizeof(unsigned int), mesh->indices, GL_STATIC_DRAW);
    }
    else
    {
        model->element_buffer_obj = 0;
    }

    // vao is unbound here but remembers info about the vbo
    // bind the vao again before drawing
    glBindVertexArray(0);
}

void model_draw(struct Model* model)
{
    glBindVertexArray(model->vertex_array_obj);

    if (model->element_buffer_obj)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->element_buffer_obj);
        glDrawElements(GL_TRIANGLES, model->mesh->indices_length, GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, model->mesh->vertices_length / 3);
    }
}

void model_release(struct Model** model)
{
    if (!*model)
    {
        return;
    }

    glDeleteBuffers(1, &(*model)->vertex_array_obj);
    glDeleteBuffers(1, &(*model)->vertex_buffer_obj);
    glDeleteBuffers(1, &(*model)->element_buffer_obj);

    mesh_release(&(*model)->mesh);

    *model = NULL;
}
