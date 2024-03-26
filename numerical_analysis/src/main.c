#include <stdlib.h>
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


int main(int argc, char* argv[])
{
#if ENABLE_MPI
    initialize_mpi(&argc, &argv);
    int rank = get_rank_mpi();
    int procs = get_procs_mpi();
    int main_proc = get_main_mpi();
#else
    int rank = 0;
    int procs = 0;
    int main_proc = 0;
#endif

    int iterations = 9;

    // If MPI is enabled and multiple processes are being used
    // only the main process does anything more than participate in solving
    if (ENABLE_MPI && procs != 1 && rank != main_proc)
    {
        //struct EquationSet eqset = {};
        //solve_equations_mpi(&eqset, iterations);
        solve_equations_mpi(NULL, iterations);
        finalize_mpi(0);
        return 0;
    }


    // Specify filepaths relative to [repository]/models/
    const char* filename = "grid2.frame";

    // Load nodes, elements and boundary conditions from file
    struct Frame frame;
    if (frame_import(filename, &frame))
    {
        finalize_mpi(0);
        return 1;
    }

    // Assign nodes a group color such that neighbors are never in the same group
    frame_assign_multicolor(&frame);

    // Produce the set of matrices and vectors representing the problem
    struct EquationSet eqset;
    frame_build_equations(&frame, &eqset);

    for (int j = 0; j < eqset.stiffness.rows; ++j)
    {
        for (int i = 0; i < eqset.stiffness.cols; ++i)
        {
            float stiffness = eqset.stiffness.elements[i + j * eqset.stiffness.cols];
            char symbol = '_';
            if (stiffness != 0.0f)
            {
                symbol = 'k';
            }

            printf("%c ", symbol);
        }

        printf("\n");
    }

    printf("\n");


    // Create space to hold the residuals
    struct vecf residuals;
    vecf_init(&residuals, iterations);

    // It seems for most boundary condition sets the stiffness matrix will not be
    // diagonally dominant so convergence is not guaranteed
    mat_diagnonal_dominance(eqset.stiff_bc);

    // Solve Equations
    if (!ENABLE_MPI || procs == 1)
    {
        // There is only one process so solve directly
        solve_jacobi_single(eqset, residuals.elements, iterations);

        // Parallel Jacobi using OpenMP
        //solve_jacobi_parallel(eqset, residuals.elements, iterations, 8);

        //solve_sor_single(eqset, residuals.elements, iterations, 1.1f);
    }
    else
    {
        // Solve using MPI (See linsolvempi.h/c)
        // Only uses Jacobi which has convergence problems
        // but does get the same result as single thread
        solve_equations_mpi(&eqset, iterations);
    }

    // Continue as normal to process and render results on the main process

    // Populate per node properties using displacements to back calculate forces
    frame_update_results(&frame, &eqset);

    // The equation set is no longer needed
    vecf_release(&residuals);
    equationset_release(&eqset);

    //frame_print_results(&frame);


    // Must be called to initialize openGL before models can be created
    struct GLFWwindow* window;
    graphics_create_window(&window);

    // Create a renderable mesh representation of the frame
    struct Mesh mesh;
    struct Model model;
    frame_create_mesh(&frame, &mesh);

    //frame_mesh_recolor(&frame, &mesh, color_disp_x);
    //frame_mesh_recolor(&frame, &mesh, color_disp_y);
    frame_mesh_recolor(&frame, &mesh, color_parallel_multicolor);

    create_model_from_mesh(&mesh, &model);

    // Render the frame
    render_model(window, &model);

    // Cleanup
    mesh_release(&mesh);
    model_release(&model);
    frame_release(&frame);

    // Render a mesh loaded from stl
    //graphics_demo(window);

    graphics_shutdown(&window);

    finalize_mpi(0);
    return 0;
}
