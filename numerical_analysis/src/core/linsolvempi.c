#include "linsolvempi.h"

#include <stdlib.h>
#include <stdio.h>

#include "mpiutility.h"
#include "frame.h"

#ifndef ENABLE_MPI
#define ENABLE_MPI 0
#endif

#if ENABLE_MPI
#include <mpi.h>
#endif

#define TAG_SIZE 1
#define TAG_MATRIX 2
#define TAG_FORCE 3
#define TAG_DISPLACEMENT 4

int solve_equations_mpi(struct EquationSet* eqset, int iterations)
{
#if ENABLE_MPI == 0
    fprintf(stderr, "Warning: Attempting to use MPI functionality with MPI disabled\n");
#else

    double startTime = MPI_Wtime();
    int rank = get_rank_mpi();
    int procs = get_procs_mpi();
    int root = get_main_mpi();

    int print = rank == 1;

    // Note only the root process has any memory allocated in the eqset
    // at the start and the vector/matrix dimensions are not set
    // so start by broadcasting the vector size
    int vec_size = 0;
    float* stiff_mat = NULL;
    float* force_vec = NULL;

    if (rank == root)
    {
        vec_size = eqset->displacements.count;
        stiff_mat = eqset->stiff_bc.elements;
        force_vec = eqset->forces.elements;
    }

    // MPI_Bcast: the root (main process) sends (broadcasts) to all processes (including itself)
    // in a communicator, but all processes in comm must call Bcast with the same root
    // to recieve. Much easier if all processes hit the same line 
    // (It can work otherwise but without tags its confusing which matches which with multiple)
    MPI_Bcast(&vec_size, 1, MPI_INT, root, MPI_COMM_WORLD);

    // All processes now have same value for vec_size
    if (print) printf("%d recieved vector size: %d\n", rank, vec_size);


    // MPI_Scatter acts like Bcast except instead of sending the whole array
    // it breaks the array into portions and sends each process a chunk (including the root)
    // The data must be evenly divisable by the number of processes in the communicator
    // See MPI_Scatterv to allow different sizes to be sent to each process
    // the send size is size of the resulting chunks not the size of source array
    // MPI_Gather works in reverse to build a full array on the root process
    // from chunks sent from each process (Both scatter and gather preserve order by rank)

    // In this case chunk size is the number of rows and force terms being used
    // we need all of the previous iterations displacement values but only update
    // a chunks worth in the current iteration

    // Create a buffer to recieve the force chunk
    // MPI_Scatter can handle even splits. will handle non divisible case later
    int chunk_size = vec_size / procs;

    if (print) printf("%d: vec: %d, chunk: %d, procs: %d\n", rank, vec_size, chunk_size, procs);

    float* force_chunk = malloc(sizeof(*force_chunk) * chunk_size);

    MPI_Scatter(force_vec, chunk_size, MPI_FLOAT, force_chunk, chunk_size, MPI_FLOAT, root, MPI_COMM_WORLD);

    // Each process now has a chunk of the force vector
    if (print) printf("%d recieved force chunk: \n", rank);

    if (print)
    {
        printf("%d: F   ", rank);
        for (int i = 0; i < chunk_size; ++i)
        {
            printf("%4.2f ", force_chunk[i]);
        }
        printf("\n");
    }

    // Now do the same for the stiffness matrix
    int chunk_size_stiff = chunk_size * vec_size;
    float* stiff_chunk = malloc(sizeof(*stiff_chunk) * chunk_size_stiff);

    MPI_Scatter(stiff_mat, chunk_size_stiff, MPI_FLOAT, stiff_chunk, chunk_size_stiff, MPI_FLOAT, root, MPI_COMM_WORLD);
    if (print) printf("%d recieved stiffness chunk: \n", rank);

    if (print)
    {
        for (int j = 0; j < chunk_size; ++j)
        {
            printf("%d: S   ", rank);
            for (int i = 0; i < vec_size; ++i)
            {
                printf("%4.2f ", stiff_chunk[i + j * vec_size]);
            }
            printf("\n");
        }
    }
    // Now the iteration begins
    // For each iteration all of the processes produce a partial result that must be gathered together
    // then the combined result broadcast to start the next iteration

    // Need space for each process to hold the previous iterations displacement values
    // and the current iterations displacement values for the chunk (could reuse but this is much simpler)
    float* prev_x = malloc(sizeof(*prev_x) * vec_size);
    float* curr_x = malloc(sizeof(*curr_x) * chunk_size);


    // Not uncommon to give an intial guess for the displacements (will revisit)
    if (rank == root)
    {
        for (int i = 0; i < vec_size; ++i)
        {
            prev_x[i] = 0; // There are several strategies for picking initial values
        }

        // Set initial guess to x_i = b_i / A_ii
        // Convergence may be faster with better initial guesses and in some cases
        // failure to converge was seen when initialized to zero
        for (int i = 0; i < vec_size; ++i)
        {
            float diag = stiff_mat[i + i * vec_size];

            // Keep initial value zero for stiffness elements eliminated by boundary conditions
            if (diag != 1.0f || diag != 0.0f)
            {
                prev_x[i] = force_vec[i] / diag;
            }
            else
            {
                prev_x[i] = 0.0f;
            }

        }
    }

    // Broadcast the initial guess
    MPI_Bcast(prev_x, vec_size, MPI_FLOAT, get_main_mpi(), MPI_COMM_WORLD);

    for (int t = 0; t < iterations; ++t)
    {
        if (print) printf("%d: Iteration: %d\n", rank, t);

        // Solve partial equation perhaps using OpenMP to parallelize further
        update_jacobi(stiff_chunk, prev_x, curr_x, force_chunk, chunk_size, vec_size);

        if (print)
        {
            printf("%d: X Prev  ", rank);
            for (int i = 0; i < vec_size; ++i)
            {
                printf("%4.2f ", prev_x[i]);
            }
            printf("\n");

            printf("%d: X Curr  ", rank);
            for (int i = 0; i < chunk_size; ++i)
            {
                printf("%4.2f ", curr_x[i]);
            }
            printf("\n");
        }


        if (rank == root)
        {
            // Gather partial solutions (recv_count is per process not total so should be chunk_size)
            MPI_Gather(curr_x, chunk_size, MPI_FLOAT, prev_x, chunk_size, MPI_FLOAT, root, MPI_COMM_WORLD);
        }
        else
        {
            // Safe to pass NULL to recievedata on non root but it doesnt change anything if you dont
            MPI_Gather(curr_x, chunk_size, MPI_FLOAT, NULL, 0, MPI_FLOAT, root, MPI_COMM_WORLD);
        }


        // Broadcast (not scatter) the full displacement array to prepare for the next iteration
        MPI_Bcast(prev_x, vec_size, MPI_FLOAT, root, MPI_COMM_WORLD);

        MPI_Barrier(MPI_COMM_WORLD);
    }

    // Copy results to equation set
    if (rank == root)
    {
        for (int i = 0; i < vec_size; ++i)
        {
            eqset->displacements.elements[i] = prev_x[i];
        }
    }

    // Free process specific resources
    free(curr_x);
    free(prev_x);
    free(stiff_chunk);
    free(force_chunk);

    double endTime = MPI_Wtime();
    if (print) printf("Solve Time: %f\n", endTime - startTime);

#endif
}


int send_equations(struct EquationSet* eqset, int dest)
{
#if ENABLE_MPI == 0
    fprintf(stderr, "Warning: Attempting to use MPI functionality with MPI disabled\n");
#else

    int rank = get_rank_mpi();

    int vec_size = eqset->displacements.count;
    int mat_size = vec_size * vec_size;

    // Look into MPI_Type_create_struct to create custom data types that can be sent in messages
    // if you need to send a struct with multiple data types like mixed float and integer
    // in this case of course the contents of the memory the pointers point to is what we want to send
    // not the addresses of the pointers and the lengths are known through count so the utility is more limited.

    // First send the degrees of freedom so recievers know how big of a buffer to allocate
    MPI_Send(&vec_size, 1, MPI_INT, dest, TAG_SIZE, MPI_COMM_WORLD);

    printf("Rank %d: Sent vector size: %d to Rank: %d\n", rank, vec_size, dest);

    // Send the stiffness matrix
    MPI_Send(eqset->stiff_bc.elements, mat_size, MPI_FLOAT, dest, TAG_MATRIX, MPI_COMM_WORLD);

    printf("Rank %d: Sent matrix\n", rank);

    // Send the forces vector
    MPI_Send(eqset->forces.elements, vec_size, MPI_FLOAT, dest, TAG_FORCE, MPI_COMM_WORLD);

    printf("Rank %d: Sent force vector\n", rank);

    // Send the displacement vector
    MPI_Send(eqset->displacements.elements, vec_size, MPI_FLOAT, dest, TAG_DISPLACEMENT, MPI_COMM_WORLD);

    printf("Rank %d: Sent displacement vector\n", rank);

    return 0;

#endif
}

int recv_equations(struct EquationSet* eqset, int src)
{
#if ENABLE_MPI == 0
    fprintf(stderr, "Warning: Attempting to use MPI functionality with MPI disabled\n");
#else

    int rank = get_rank_mpi();

    MPI_Status status = {};

    int vec_size = 0;
    MPI_Recv(&vec_size, 1, MPI_INT, src, TAG_SIZE, MPI_COMM_WORLD, &status);

    printf("Rank %d: Recieved vector size: %d\n", rank, vec_size);

    // Recieve the stiffness matrix
    int mat_size = vec_size * vec_size;
    matrix_init(&eqset->stiff_bc, vec_size, vec_size, 0);
    MPI_Recv(eqset->stiff_bc.elements, mat_size, MPI_FLOAT, src, TAG_MATRIX, MPI_COMM_WORLD, &status);

    printf("Rank %d: Recieved matrix\n", rank);


    // Recieve the force vector
    vecf_init(&eqset->forces, vec_size);
    MPI_Recv(eqset->forces.elements, vec_size, MPI_FLOAT, src, TAG_FORCE, MPI_COMM_WORLD, &status);

    printf("Rank %d: Recieved force vector\n", rank);


    // Recieve the displacement vector
    vecf_init(&eqset->displacements, vec_size);
    MPI_Recv(eqset->displacements.elements, vec_size, MPI_FLOAT, src, TAG_DISPLACEMENT, MPI_COMM_WORLD, &status);

    printf("Rank %d: Recieved displacement vector\n", rank);

    return 0;

#endif
}


void update_jacobi(float* matrix_a, float* prev_x, float* curr_x, float* vector_b, int rows, int cols)
{
#if ENABLE_MPI == 0
    fprintf(stderr, "Warning: Attempting to use MPI functionality with MPI disabled\n");
#else
    // Inputs are generalized. for structures the matrix is stiffness, x is displacement, and b is force

    //int procs = get_procs_mpi();
    int rank = get_rank_mpi();

    int print = rank == 1;

    // Revisit when dealing with uneven chunk sizes
    /*// ensures we essentially "round up" after a division of rows / procs
    // so 7 / 8 => 1, 7 / 6 => 2
    // this ensures that chunksize is the smallest number where each process
    // gets at most chunksize rows to work on
    int chunksize = (rows + procs - 1) / procs;
    // The first row in the chunk
    int rowstart = chunksize * rank;
    // One past the last row in the chunk clamped to be no more than rows
    int rowend = rowstart + chunksize;
    rowend = rowend > rows ? rowend : rows; */


    // Instead of recieving the whole matrix each process should only recieve the chunk it will work on
    // rows is equivalent to chunksize or force/curr_x length and cols is equivalent to prev_x length

    // update all of the rows inside the current chunk
    for (int j = 0; j < rows; ++j)
    {
        // sum up A_ij * x_i as long as i != j
        float x = 0;
        for (int i = 0; i < cols; ++i)
        {
            if (i != (j + rank * rows))
            {
                x += matrix_a[i + j * cols] * prev_x[i];
            }
        }

        if (print) printf("%d: ", rank);
        if (print) printf("Sum: %f, ", x);

        // subtract the sum from b_j
        x = vector_b[j] - x;

        if (print) printf("F-Sum: %f, ", x);

        // divide by the corresponding diagonal A_jj that was skipped in summation
        float a_jj = matrix_a[j + j * cols + rank * rows];

        if (print) printf("Diag: %f, ", a_jj);

        // if you ensure diagonals are set to 1 if not active this check can be skipped
        if (a_jj != 0.f)
        {
            x /= a_jj;
        }

        if (print) printf("X: %f\n", x);

        // update x_j to the new estimate
        curr_x[j] = x;
    }

#endif
}
