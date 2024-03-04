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

    model->transform = MAT4_IDENTITY;

    // OpenGL resources are identified by id number

    // Generate and bind a Vertex Array Object (to store attributes)
    // While bound, Vertex Array Object stores information about the bound Vertex Buffer Object
    // as calls are made to modify properties on the VBO

    unsigned int obj = model->vertex_array_obj;
    unsigned int* pobj = &obj;

    glGenVertexArrays(1, &model->vertex_array_obj);
    glBindVertexArray(model->vertex_array_obj);

    // Generate and bind a vertex buffer object (to store actual vertex data)
    // any calls targeting GL_ARRAY_BUFFER will effect this vbo until it is unbound
    glGenBuffers(1, &model->vertex_buffer_obj);
    glBindBuffer(GL_ARRAY_BUFFER, model->vertex_buffer_obj);

    // Copy vertex data to gpu memory
    glBufferData(GL_ARRAY_BUFFER, mesh->vertices_length * sizeof(struct Vertex), mesh->vertices, GL_STATIC_DRAW);

    // Tell the gpu how the vertex buffer data is laid out (vbo is still bound) this can be done manually
    // each time the vbo is bound or use the vao to store this info and use it when the vao is bound
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Attribute layout for vertex color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

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

    model->cull_backface = 0;
    model->draw_wireframe = 0;
}

void model_draw(struct Model* model, unsigned int transform_id)
{
    // Enable/Disable back face culling
    if (model->cull_backface)
    {
        glEnable(GL_CULL_FACE);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }

    // Select Fill/Wireframe
    if (model->draw_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Default
    }

    // Set the shader transform uniform
    glUniformMatrix4fv(transform_id, 1, GL_FALSE, (float*)&model->transform);

    // Bind the vertex array object (implicitly binds vertex buffer object)
    glBindVertexArray(model->vertex_array_obj);

    // If the model has indices bind the element buffer object and draw as elements
    // otherwise draw as an array of triangles (basically draw as if indices were in order)
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

void model_release(struct Model* model)
{
    if (!model)
    {
        return;
    }

    glDeleteBuffers(1, &model->vertex_array_obj);
    glDeleteBuffers(1, &model->vertex_buffer_obj);
    glDeleteBuffers(1, &model->element_buffer_obj);

    // Model is not responsible for the lifetime of the mesh since they may be shared
    model->mesh = NULL;
}
