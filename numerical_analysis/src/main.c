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
#include "linearsolve.h"


int main(int argc, char* argv[])
{
    /* initialize_mpi(&argc, &argv);

    //hot_potato(get_rank_mpi(), get_procs_mpi());

    // Wait until time to solve the linear equation then return
    // to single process for rendering
    if (get_rank_mpi() != 0)
    {
        wait_broadcast(get_rank_mpi(), get_procs_mpi());
        finalize_mpi(0);
        return 0;
    } */

    //finiteDifference1D();
    //finiteDifference2D();

    // Must be called to initialize openGL before models can be created
    struct GLFWwindow* window;
    graphics_create_window(&window);

    // Specify filepaths relative to [repository]/models/
    //const char* filename = "beam.frame";
    //const char* filename = "bicycle.frame";
    const char* filename = "car.frame";

    // Load nodes, elements and boundary conditions from file
    struct Frame frame;
    if (frame_import(filename, &frame))
    {
        graphics_shutdown(&window);
        return 1;
    }

    // Produce the matrices and vectors needed to solve
    struct EquationSet eqset;
    frame_build_equations(&frame, &eqset);

    // This is where you could send the data needed to start solving on other processes
    /* if (get_rank_mpi() == 0)
    {
        send_broadcast(get_procs_mpi());
    }*/

    // This is still using OpenMP but the necessary data is now more exposed
    // to passing around with MPI in the future
    // {
    // Solve F = KU for displacements U = k^-1 * F
    // using the known boundary condition forces
    int dof_count = 6 * frame.node_count;
    solve_jacobi(eqset.stiff_bc.elements, eqset.displacements.elements,
        eqset.forces.elements, dof_count, dof_count, 100, 1);
    // }


    // Populate per node properties using displacements to back calculate forces
    frame_update_results(&frame, &eqset);

    // The equation set is no longer needed
    equationset_release(&eqset);

    // Old method with OpenMP where everything is encapsulated
    //frame_solve(&frame);

    // Create a renderable mesh representation of the frame
    struct Mesh mesh;
    struct Model model;
    frame_create_mesh(&frame, &mesh);
    create_model_from_mesh(&mesh, &model);

    // Render the frame with color coded displacement
    render_model(window, &model);

    // Or load a mesh from stl to view
    // Specify filepaths relative to [repository]/models/
    //create_sample_stl(&mesh, &model, "orientation.stl");
    //create_sample_stl(&mesh, &model, "3DBenchy.stl");
    //create_sample_grid(&mesh, &model);
    // Render the model
    //render_model(window, &model);


    // Cleanup
    mesh_release(&mesh);
    model_release(&model);
    frame_release(&frame);

    graphics_shutdown(&window);

    //finalize_mpi(0);
    return 0;
}
