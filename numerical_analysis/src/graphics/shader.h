#pragma once

// Basic vertex shader
extern const char* BASIC_VS_SRC;

// Basic fragment shader
extern const char* BASIC_FS_SRC;

// Vertex color vertex shader
extern const char* VCOLOR_VS_SRC;

// Vertex color fragment shader
extern const char* VCOLOR_FS_SRC;

void vertex_shader_init(unsigned int* shaderId, const char* source);

void fragment_shader_init(unsigned int* shaderId, const char* source);

void shader_init(unsigned int* shader_program_id, const char* vert_shader_source, const char* frag_shader_source);