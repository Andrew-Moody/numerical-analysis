#include "linsolvempi.h"

#include <stdlib.h>
#include <stdio.h>

#include "mpiutility.h"

#ifndef ENABLE_MPI
#define ENABLE_MPI 0
#endif

#if ENABLE_MPI
#include <mpi.h>
#endif

#if ENABLE_MPI == 0

void solve_jacobi_mpi(float* matrix_a, float* vector_x, float* vector_b, int rows, int cols, int iterations)
{
    fprintf(stderr, "Warning: Attempting to use MPI functionality with MPI disabled\n");
}

#else


void solve_jacobi_mpi(float* matrix_a, float* vector_x, float* vector_b, int rows, int cols, int iterations)
{
    double startTime = MPI_Wtime();

    //fprintf(stderr, "Warning: Attempting to use MPI functionality with MPI disabled\n");

    int procs = get_procs_mpi();
    int rank = get_rank_mpi();

    // ensures we essentially "round up" after a division of rows / procs
    // so 7 / 8 => 1, 7 / 6 => 2
    // this ensures that chunksize is the smallest number where each process
    // gets at most chunksize rows to work on
    int chunksize = (rows + procs - 1) / procs;

    // The first row in the chunk
    int rowstart = chunksize * rank;
    // One past the last row in the chunk clamped to be no more than rows
    int rowend = rowstart + chunksize;
    rowend = rowend > rows ? rowend : rows;


    // Instead of recieving the whole matrix each process should only recieve the chunk it will work on

    for (int t = 0; t < iterations; ++t)
    {
        // update all of the rows inside the current chunk
        for (int j = rowstart; j < rowend; ++j)
        {
            // sum up A_ij * x_i as long as i != j
            float x = 0;
            for (int i = 0; i < cols; ++i)
            {
                if (i != j)
                {
                    x += matrix_a[i + j * cols] * vector_x[i];
                }
            }

            // subtract the sum from b_j
            x = vector_b[j] - x;

            // divide by the corresponding diagonal A_jj that was skipped in summation
            float a_jj = matrix_a[j + j * cols];

            // if you ensure diagonals are set to 1 if not active this check can be skipped
            if (a_jj != 0.f)
            {
                x /= a_jj;
            }

            // update x_j to the new estimate
            vector_x[j] = x;
        }




        //omp

        for (int j = 0; j < rows; ++j)
        {
            // sum up A_ij * x_i as long as i != j
            float x = 0;
            for (int i = 0; i < cols; ++i)
            {
                if (i != j)
                {
                    x += matrix_a[i + j * cols] * vector_x[i];
                }
            }

            // subtract the sum from b_j
            x = vector_b[j] - x;

            // divide by the corresponding diagonal A_jj that was skipped in summation
            float a_jj = matrix_a[j + j * cols];

            // if you ensure diagonals are set to 1 if not active this check can be skipped
            if (a_jj != 0.f)
            {
                x /= a_jj;
            }

            // update x_j to the new estimate
            vector_x[j] = x;
        }
    }

    double endTime = MPI_Wtime();

    printf("Solve Time: %f\n", endTime - startTime);

}


#endif