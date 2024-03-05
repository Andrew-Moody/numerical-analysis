#include "application.h"

#include <stdlib.h>

#include "camera.h"

void application_init(struct Application* application)
{
    application->camera = malloc(sizeof(*application->camera));
    camera_init(application->camera, 800, 600, 0.f, 10.f, 4.0f);
}

void application_release(struct Application* application)
{
    free(application->camera);
}