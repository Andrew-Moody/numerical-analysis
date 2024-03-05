#pragma once

struct Camera;

struct Application
{
    struct Camera* camera;
    unsigned int proj_matrix_id;
};

void application_init(struct Application* application);

void application_release(struct Application* application);