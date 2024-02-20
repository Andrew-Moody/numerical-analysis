#include "camera.h"

void camera_init(struct Camera* camera, int screen_width, int screen_height, float near, float far, float zoom)
{
    camera->screen_width = screen_width;
    camera->screen_height = screen_height;
    camera->near_z = near;
    camera->far_z = far;
    camera->zoom = zoom;

    camera->transform = MAT4_IDENTITY;

    // Mapping physical units (say meters) to normalized device coordinates (-1, 1)

    float aspect = screen_width / (float)screen_height;

    // for now let physical screen height be 1 meter * zoom fraction
    float height = zoom;
    float width = aspect * height;


    // Scaling
    float sx = 2.f / width;
    float sy = 2.f / height;
    float sz = -2.f / (far - near);

    // Translation
    float px = 0.0f;
    float py = 0.0f;
    float pz = (far - near) / (far + near);

    camera->proj_matrix = mat4_transform(px, py, pz, 0.f, 0.f, 0.f, sx, sy, sz);
}
