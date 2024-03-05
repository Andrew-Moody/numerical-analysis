#include <stdlib.h>
#include <stdio.h>

#include "graphics.h"
#include "fluid.h"
#include "frame.h"
#include "mesh.h"
#include "model.h"

int main(void)
{

    //finiteDifference1D();
    //finiteDifference2D();

    // Must be called to initialize openGL before models can be created
    struct GLFWwindow* window;
    graphics_create_window(&window);

    // Render a grid, or imported STL mesh
    struct Mesh mesh;
    struct Model model;
    //create_sample_stl(&mesh, &model, "../../models/orientation.stl");
    create_sample_stl(&mesh, &model, "../../models/3DBenchy.stl");
    //create_sample_grid(&mesh, &model);


    // Create and solve a structural frame 
    struct Frame frame;
    frame_init(&frame);

    frame_solve(&frame);

    // Create a renderable mesh representation of the frame
    //frame_create_mesh(&frame, &mesh);
    //create_model_from_mesh(&mesh, &model);

    render_model(window, &model);

    // Cleanup
    mesh_release(&mesh);
    model_release(&model);
    frame_release(&frame);

    graphics_shutdown(&window);

    return 0;
}
