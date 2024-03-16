#include "linearsolve.h"

#include <stdio.h>
#include <omp.h>

#include "frame.h"

void solve_jacobi_omp(struct EquationSet* eqset, int iterations, int desired_threads)
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

    float* matrix_a = eqset->stiff_bc.elements;
    float* vector_x = eqset->displacements.elements;
    float* vector_b = eqset->forces.elements;
    int rows = eqset->stiff_bc.rows;
    int cols = eqset->stiff_bc.cols;


    if (desired_threads < 1)
    {
        // Most likely not intended
        printf("Warning: Attempted to solve with desired_threads less than 1");
        return;
    }
    else if (desired_threads == 1)
    {
        int print = 1;


        // Need a place to store values from previous iteration to not clobber them while updating current iteration
        float* prev_x = malloc(sizeof(*prev_x) * cols);

        // Set initial guess to x_i = b_i / A_ii
        // Convergence may be faster with better initial guesses and in some cases
        // failure to converge was seen when initialized to zero
        for (int i = 0; i < cols; ++i)
        {
            float diag = matrix_a[i + i * cols];

            // Keep initial value zero for stiffness elements eliminated by boundary conditions
            if (diag != 1.0f || diag != 0.0f)
            {
                vector_x[i] = vector_b[i] / diag;
            }
            else
            {
                vector_x[i] = 0.0f;
            }

        }

        // Single threaded only
        for (int t = 0; t < iterations; ++t)
        {
            // update to use the newly calculated values
            for (int i = 0; i < cols; ++i)
            {
                prev_x[i] = vector_x[i];
            }

            if (print) printf("%d: Iteration: %d\n", 0, t);

            // update one row at a time
            for (int j = 0; j < rows; ++j)
            {
                // sum up A_ij * x_i as long as i != j
                float x = 0;
                for (int i = 0; i < cols; ++i)
                {
                    if (i != j)
                    {
                        x += matrix_a[i + j * cols] * prev_x[i];
                    }
                }

                if (print) printf("%d: ", 0);
                if (print) printf("Sum: %f, ", x);

                // subtract the sum from b_j and divide by A_jj
                x = vector_b[j] - x;

                if (print) printf("F-Sum: %f, ", x);

                // if you ensure diagonals are set to 1 if not active this check can be skipped
                float k_jj = matrix_a[j + j * cols];

                if (print) printf("Diag: %f, ", k_jj);

                if (k_jj != 0.f)
                {
                    x /= k_jj;
                }

                if (print) printf("X: %f\n", x);

                // update x_j to the new estimate
                vector_x[j] = x;
            }

            if (print)
            {
                printf("%d: X Prev ", 0);
                for (int i = 0; i < cols; ++i)
                {
                    printf("%4.2f ", prev_x[i]);
                }
                printf("\n");

                printf("%d: X Curr ", 0);
                for (int i = 0; i < cols; ++i)
                {
                    printf("%4.2f ", vector_x[i]);
                }
                printf("\n");
            }
        }

        free(prev_x);
    }
    else
    {
        // This is technically Gauss-Seidel not Jacobi since the x value being currently updated
        // takes into account x values that have already been updated this iteration
        // this seems to work since omp threads share memory but doesn't translate well to MPI as is

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
