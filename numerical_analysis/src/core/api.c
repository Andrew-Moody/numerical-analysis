#include "api.h"

#include <stdio.h>

#include "graphics.h"
#include "fluid.h"
#include "frame.h"
#include "mesh.h"
#include "model.h"
#include "frameimport.h"
#include "frameprocess.h"

#include "mpiutility.h"
#include "mpitest.h"
#include "linsolvempi.h"
#include "linearsolve.h"

void run_demo(const char* filename)
{
    fputs(filename, stdout);
    fputc('\n', stdout);
    fflush(stdout);

    // Import a frame model from a file
    struct Frame frame;
    if (frame_import(filename, &frame))
    {
        fprintf(stderr, "Error: Failed to import frame with filename: %s\n", filename);
        return;
    }

    // Assign nodes a group color such that neighbors are never in the same group
    frame_assign_multicolor(&frame);

    // Produce the set of matrices and vectors representing the problem
    struct EquationSet eqset;
    frame_build_equations(&frame, &eqset);

    int iterations = 100;

    // Create space to hold the residuals
    struct vecf residuals;
    vecf_init(&residuals, iterations);

    // Solve the system representing the frame
    solve_sor_single(eqset, residuals.elements, iterations, 1.1f);

    // Populate per node properties using displacements to back calculate forces
    frame_update_results(&frame, &eqset);

    // The equation set is no longer needed
    vecf_release(&residuals);
    equationset_release(&eqset);

    // Must be called to initialize openGL before models can be created
    struct GLFWwindow* window;
    graphics_create_window(&window);

    // Create a renderable mesh representation of the frame
    struct Mesh mesh;
    struct Model model;
    frame_create_mesh(&frame, &mesh);

    //frame_mesh_recolor(&frame, &mesh, color_disp_x);
    frame_mesh_recolor(&frame, &mesh, color_disp_y);
    //frame_mesh_recolor(&frame, &mesh, color_parallel_multicolor);

    create_model_from_mesh(&mesh, &model);

    // Render the frame
    render_model(window, &model);

    // Cleanup
    mesh_release(&mesh);
    model_release(&model);
    frame_release(&frame);

    graphics_shutdown(&window);
}