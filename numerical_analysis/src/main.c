#include <stdlib.h>
#include <stdio.h>

#include "graphics.h"
#include "fluid.h"
#include "frame.h"
#include "mesh.h"
#include "model.h"
#include "frameimport.h"

#include "mpiutility.h"
#include "mpitest.h"
#include "linsolvempi.h"
#include "linearsolve.h"


int main(int argc, char* argv[])
{
    initialize_mpi(&argc, &argv);
    int rank = get_rank_mpi();
    int procs = get_procs_mpi();
    int main_proc = get_main_mpi();
    int iterations = 10;

    // If MPI is enabled and multiple processes are being used
    // all but the main process participate only in solving
    if (ENABLE_MPI && procs != 1 && rank != main_proc)
    {
        struct EquationSet eqset = {};
        solve_equations_mpi(&eqset, iterations);

        /* int vec_size;
        MPI_Bcast(&vec_size, 1, MPI_INT, main_proc, MPI_COMM_WORLD);
        printf("%d recieved vector size: %d\n", rank, vec_size);
        // Recieve equation set, solve, send back results, then return
        // to single process for rendering

        struct EquationSet eqset = {};
        recv_equations(&eqset, main_proc);

        // This is still using OpenMP to solve on a single process
        // Solve F = KU for displacements U = k^-1 * F
        // using the known boundary condition forces
        solve_jacobi_omp(&eqset, 100, 1);

        send_equations(&eqset, main_proc); */

        finalize_mpi(0);
        return 0;
    }


    // Specify filepaths relative to [repository]/models/
    const char* filename = "car.frame";

    // Load nodes, elements and boundary conditions from file
    struct Frame frame;
    if (frame_import(filename, &frame))
    {
        finalize_mpi(0);
        return 1;
    }

    // Produce the set of matrices and vectors representing the problem
    struct EquationSet eqset;
    frame_build_equations(&frame, &eqset);

    printf("\n");

    //matrix_print(eqset.stiff_bc);

    printf("\n");

    // It seems for most boundary condition sets the stiffness matrix will not be
    // diagonally dominant even when not counting eliminated elements
    mat_diagnonal_dominance(eqset.stiff_bc);

    printf("\n");

    // Solve Equations
    if (!ENABLE_MPI || procs == 1)
    {
        // There is only one process so just solve directly
        struct vecf residuals;
        vecf_init(&residuals, iterations);

        struct vecf prev_x;
        vecf_init(&prev_x, eqset.displacements.count);

        struct EquationChunk chunk = {
            eqset.stiff_bc.elements,
            eqset.forces.elements,
            eqset.displacements.elements,
            prev_x.elements,
            eqset.stiff_bc.rows,
            eqset.stiff_bc.cols
        };

        solve_jacobi_single(chunk, residuals.elements, iterations);

        for (int i = 0; i < iterations; ++i)
        {
            printf("%d: %f\n", i, residuals.elements[i]);
        }
    }
    else
    {
        // Solve using MPI (See linsolvempi.h/c)
        solve_equations_mpi(&eqset, iterations);
    }

    // Continue as normal to process and render results

    // Populate per node properties using displacements to back calculate forces
    frame_update_results(&frame, &eqset);

    // The equation set is no longer needed
    equationset_release(&eqset);

    //frame_print_results(&frame);

    // Old method with OpenMP where everything is encapsulated
    //frame_solve(&frame);

    // Must be called to initialize openGL before models can be created
    struct GLFWwindow* window;
    graphics_create_window(&window);

    // Create a renderable mesh representation of the frame
    struct Mesh mesh;
    struct Model model;
    frame_create_mesh(&frame, &mesh);
    create_model_from_mesh(&mesh, &model);

    // Render the frame with color coded displacement
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
