#include "linearsolve.h"

#include <stdio.h>
#include <omp.h>


void solve_jacobi(float* matrix_a, float* vector_x, float* vector_b, int rows, int cols, int iterations, int desired_threads)
{
    // Solves a linear system of the form Ax = b for x using the Jacobi Method

    // It uses an iterative approach where each step uses the previous estimate to
    // give an improved estimate with the error between estimate and exact solution
    // converging towards zero as iterations increase

    // The new estimate for x_i (the ith value in the x vector) at iteration k+1 is given by:

    // x_i( k+1 ) = ( 1/A_ii ) * ( b_i - sum( A_ij * x_i( k ) ) ) for j = 0 to cols-1 where j != i

    // Within a single iteration step this method does not read any data that is also being written to
    // unlike Gauss-Seidel or Successive Over Relaxation so it is easier to implement in parallel
    // but can be much slower if using only a single thread

    double startTime = omp_get_wtime();

    if (desired_threads < 1)
    {
        // Most likely not intended
        printf("Warning: Attempted to solve with desired_threads less than 1");
        return;
    }
    else if (desired_threads == 1)
    {
        // Single threaded only
        for (int t = 0; t < iterations; ++t)
        {
            // update one row at a time
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

                // subtract the sum from b_j and divide by A_jj
                x = vector_b[j] - x;

                // if you ensure diagonals are set to 1 if not active this check can be skipped
                float k_jj = matrix_a[j + j * cols];
                if (k_jj != 0.f)
                {
                    x /= k_jj;
                }

                // update x_j to the new estimate
                vector_x[j] = x;
            }
        }
    }
    else
    {
        // Multi-threaded
#pragma omp parallel num_threads(desired_threads)
        // for (int t = 0; t < 10; ++t) dohh
        for (int t = 0; t < iterations; ++t)
        {
            // update multiple rows at a time
#pragma omp for schedule(dynamic) nowait
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

                // subtract the sum from b_j and divide by A_jj
                x = vector_b[j] - x;

                // if you ensure diagonals are set to 1 if not active this check can be skipped
                float k_jj = matrix_a[j + j * cols];
                if (k_jj != 0.f)
                {
                    x /= k_jj;
                }

                // update x_j to the new estimate
                vector_x[j] = x;
            }
        }
    }

    double endTime = omp_get_wtime();

    printf("Solve Time: %f\n", endTime - startTime);
}
