#include "camera.h"

void camera_init(struct Camera* camera, int screen_width, int screen_height, float near, float far, float zoom)
{
    camera->screen_width = screen_width;
    camera->screen_height = screen_height;
    camera->near_z = near;
    camera->far_z = far;
    camera->zoom = zoom;

    camera->position = (struct vec3){ 0.f, 0.f, 5.f };
    camera->rotation = (struct vec3){ 0.f, 0.f, 0.f };
    camera->transform = MAT4_IDENTITY;

    // Mapping physical units (say meters) to normalized device coordinates (-1, 1)

    float aspect = screen_width / (float)screen_height;

    // for now let physical screen height be 1 meter * zoom fraction
    float height = zoom;
    float width = aspect * height;


    // Scaling
    /* float sx = 2.f / width;
    float sy = 2.f / height;
    float sz = -2.f / (far - near); */

    float sx = 1.f / width;
    float sy = 1.f / height;
    float sz = -2.f / (far - near);

    // Translation
    float px = 0.0f;
    float py = 0.0f;
    float pz = (far - near) / (far + near);

    camera->proj_matrix = mat4_transform(px, py, pz, 0.f, 0.f, 0.f, sx, sy, sz);
}


void camera_update(struct Camera* camera)
{
    camera->transform = mat4_transform(camera->position.x, camera->position.y, camera->position.z,
        camera->rotation.x, camera->rotation.y, camera->rotation.z, 1.f, 1.f, 1.f);

    camera->view_transform = mat4_multiply(camera->transform, camera->proj_matrix);
}

void camera_translate(struct Camera* camera, float x, float y, float z)
{
    camera->position.x += x;
    camera->position.y += y;
    camera->position.z += z;
}

void camera_rotate(struct Camera* camera, float x, float y, float z)
{
    camera->rotation.x += x;
    camera->rotation.y += y;
    camera->rotation.z += z;
}
