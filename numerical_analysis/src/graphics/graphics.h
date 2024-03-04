#pragma once

struct Mesh;
struct Model;
struct GLFWwindow;

void create_sample_grid(struct Mesh* mesh, struct Model* model);

void create_sample_stl(struct Mesh* mesh, struct Model* model);

void create_model_from_mesh(struct Mesh* mesh, struct Model* model);

void render_model(struct GLFWwindow* window, struct Model* model);

int graphics_create_window(struct GLFWwindow** window);

void graphics_shutdown(struct GLFWwindow** window);
