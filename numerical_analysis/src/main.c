#include <stdlib.h>
#include <stdio.h>

#include "graphics.h"
#include "fluid.h"
#include "frame.h"
#include "mesh.h"
#include "model.h"
#include "frameimport.h"
#include "mpitest.h"


int main(int argc, char* argv[])
{

    mpi_test(&argc, &argv);

    //finiteDifference1D();
    //finiteDifference2D();

    // Must be called to initialize openGL before models can be created
    struct GLFWwindow* window;
    graphics_create_window(&window);

    // Render a grid, or imported STL mesh
    struct Mesh mesh;
    struct Model model;
    // Specify filepaths relative to [repository]/models/
    //create_sample_stl(&mesh, &model, "orientation.stl");
    //create_sample_stl(&mesh, &model, "3DBenchy.stl");
    //create_sample_grid(&mesh, &model);


    // Load nodes, elements and boundary conditions from file
    struct Frame frame;
    // Specify filepaths relative to [repository]/models/
    //const char* filename = "beam.frame";
    //const char* filename = "bicycle.frame";
    const char* filename = "car.frame";

    if (frame_import(filename, &frame))
    {
        graphics_shutdown(&window);
        return 1;
    }

    // Solve linear equation using iterative method
    frame_solve(&frame);

    // Create a renderable mesh representation of the frame
    frame_create_mesh(&frame, &mesh);
    create_model_from_mesh(&mesh, &model);

    // Render the frame with color coded displacement
    render_model(window, &model);

    // Cleanup
    mesh_release(&mesh);
    model_release(&model);
    frame_release(&frame);

    graphics_shutdown(&window);

    return 0;
}
