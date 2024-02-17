#include "shader.h"

#include <stdlib.h>
#include <stdio.h>

// glad must be included before glfw
#include <glad/glad.h>
#include <GLFW/glfw3.h>


// Basic vertex shader
const char* BASIC_VS_SRC = "#version 420 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 transform;\n"
"void main() { gl_Position = transform * vec4(aPos.x, aPos.y, aPos.z, 1.0); }\0";

// Basic fragment shader
const char* BASIC_FS_SRC = "#version 420 core\n"
"out vec4 FragColor;\n"
"void main() { FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f); }\0";


// Vertex color vertex shader
const char* VCOLOR_VS_SRC = "#version 420 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 Color;\n"
"out vec3 VertexColor;\n"
"uniform mat4 transform;\n"
"void main() {\n"
"gl_Position = transform * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"VertexColor = Color;\n"
"}\0";

// Vertex color fragment shader
const char* VCOLOR_FS_SRC = "#version 420 core\n"
"out vec4 FragColor;\n"
"in vec3 VertexColor;\n"
"void main() { FragColor = vec4(VertexColor, 1.0f); }\0";

void vertex_shader_init(unsigned int* shaderId, const char* source)
{
    *shaderId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(*shaderId, 1, &source, NULL);
    glCompileShader(*shaderId);

    int success;
    glGetShaderiv(*shaderId, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(*shaderId, 512, NULL, infoLog);
        fprintf(stderr, "Vertex Shader compilation failed: %s\n", infoLog);
    }
}

void fragment_shader_init(unsigned int* shaderId, const char* source)
{
    *shaderId = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(*shaderId, 1, &source, NULL);
    glCompileShader(*shaderId);

    int success;
    glGetShaderiv(*shaderId, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(*shaderId, 512, NULL, infoLog);
        fprintf(stderr, "Fragment Shader compilation failed: %s\n", infoLog);
    }
}

void shader_init(unsigned int* shader_program_id, const char* vert_shader_source, const char* frag_shader_source)
{
    GLuint vertex_shader_id;
    vertex_shader_init(&vertex_shader_id, vert_shader_source);

    GLuint fragment_shader_id;
    fragment_shader_init(&fragment_shader_id, frag_shader_source);

    // Link the vertex and fragment shaders into a shader program
    *shader_program_id = glCreateProgram();
    glAttachShader(*shader_program_id, vertex_shader_id);
    glAttachShader(*shader_program_id, fragment_shader_id);
    glLinkProgram(*shader_program_id);

    int success;
    glGetProgramiv(*shader_program_id, GL_LINK_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(*shader_program_id, 512, NULL, infoLog);
        printf("Shader linking failed: %s\n", infoLog);
    }

    // Individual Shaders aren't needed after linking (unless they are reused in multiple programs)
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);
}