#include "linearsolve.h"

#include <stdio.h>
#include <omp.h>

#include "frame.h"

#define PRINT_DEBUG 1

#if PRINT_DEBUG
#define DLOG(...) (printf(__VA_ARGS__))
#else
#define DLOG(X)
#endif


float residual(float* x_vector, int x_length)
{

}

// Solve the equation set using the Jacobi iterative method
void solve_jacobi_single(struct EquationChunk chunk, float* residuals, int iterations)
{
    // Solves a linear system of the form Ax = b for x using the Jacobi Iterative Method

    // It uses an iterative approach where each step uses the previous estimate to
    // give an improved estimate with the error between estimate and exact solution
    // converging towards zero as iterations increase

    // Within a single iteration step this method does not read any data that is also being written to
    // unlike Gauss-Seidel or Successive Over Relaxation so it is easier to implement in parallel

    // Unfortunately the requirements for guaranteed convergence are more strict than for Gauss-Seidel
    // or SOR and converges slower in general



    // The new estimate for x_i (the ith value in the x vector) at iteration k+1 is given by:

    // x_i( k+1 ) = ( 1/A_ii ) * ( b_i - sum( A_ij * x_i( k ) ) ) for j = 0 to cols-1 where j != i

    // The exact error between the current approximation and the exact solution can't be known
    // without knowing the exact solution, but the change in x from one iteration to the next
    // can be used to gauge how close to the exact solution we are

    // The residual vector at iteration k is r_k = b - A * x_k
    // r_i( k ) = b_i - sum( A_ij * x_i( k ) ) for j = 0 to cols-1
    // the norm of the residual vector gives a single value each iteration that can be checked if
    // it has met some tolerance criteria or plotted to give an indication of convergence progress

    // To skip needing to check if j != i in a tight loop we just multiple the entire row times x
    // and later subtract the term we want to skip. The residual needs the full value anyway



    for (int t = 0; t < iterations; ++t)
    {
        float sum_sqr_residual = 0;

        for (int j = 0; j < chunk.rows; ++j)
        {
            // Multiply the jth row of A times the previous x vector
            // 

            // sum up A_ij * x_i as long as i != j
            /* float x = 0;
            for (int i = 0; i < chunk.cols; ++i)
            {
                if (i != (j + chunk.id * chunk.rows))
                {
                    x += chunk.matrix_a[i + j * chunk.cols] * chunk.prev_x[i];
                }
            } */

            // Since we will need the full sum anyway for the residual,
            // skip the if i != j check and subtract A_ii * x_i afterwards
            float ax = 0;
            for (int i = 0; i < chunk.cols; ++i)
            {
                ax += chunk.matrix_a[i + j * chunk.cols] * chunk.prev_x[i];
            }


            DLOG("%d: ", chunk.id);
            DLOG("Sum: %f, ", ax);

            // Subtract the sum from b_j to get the residual
            float residual = chunk.vector_b[j] - ax;

            // Add the square residual to the running sum of square residuals
            sum_sqr_residual += residual * residual;

            DLOG("Residual: %f, ", residual);

            // get the diagonal term for the row
            float a_jj = chunk.matrix_a[j + j * chunk.cols + chunk.id * chunk.rows];

            DLOG("Diag: %f, ", a_jj);

            // a_jj needs to be non-zero but this should have been ensured
            // when applying boundary conditions so check should not be needed
            /* if (a_jj == 0.f)
            {
                a_jj = 1.0f;
            } */

            // Add x_j * a_jj and divide the result by a_jj to solve for the new estimate of x_j
            float x = (residual + a_jj * chunk.prev_x[j]) / a_jj;

            DLOG("X: %f\n", x);

            // update x_j to the new estimate
            chunk.curr_x[j] = x;
        }

        float norm_residial = sqrt(sum_sqr_residual);

        residuals[t] = norm_residial;
    }
}


void solve_jacobi_parallel(struct EquationChunk chunk, float* residuals, int iterations, int desired_threads)
{
    if (desired_threads < 1)
    {
        // Most likely not intended
        printf("Warning: Attempted to solve with desired_threads less than 1");
        return;
    }
    else if (desired_threads == 1)
    {
        // Use the single threaded solution instead
        solve_jacobi_single(chunk, residuals, iterations);
        return;
    }

    float* matrix_a = chunk.matrix_a;
    float* vector_x_prev = chunk.prev_x;
    float* vector_x_curr = chunk.prev_x;
    float* vector_b = chunk.vector_b;
    int rows = chunk.rows;
    int cols = chunk.cols;

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
                    x += matrix_a[i + j * cols] * vector_x_prev[i];
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
            vector_x_curr[j] = x;
        }
    }
}


void update_chunk_jacobi(struct EquationChunk chunk)
{
    // Perform one iteration on a partial data set or chunk made up of rows from the stiffness matrix
    // and subset of the force vector. The vector of previous x values is still full length since all 
    // previous values are needed. The length of prev_x is the same as the number of columns
    // the updated values are written to curr_x since we need to control whether to use only previous values
    // or allow updating 
    // Inputs are generalized. for structures the matrix is stiffness, x is displacement, and b is force

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
    for (int j = 0; j < chunk.rows; ++j)
    {
        // sum up A_ij * x_i as long as i != j
        float x = 0;
        for (int i = 0; i < chunk.cols; ++i)
        {
            if (i != (j + chunk.id * chunk.rows))
            {
                x += chunk.matrix_a[i + j * chunk.cols] * chunk.prev_x[i];
            }
        }

        DLOG("%d: ", chunk.id);
        DLOG("Sum: %f, ", x);

        // subtract the sum from b_j
        x = chunk.vector_b[j] - x;

        DLOG("F-Sum: %f, ", x);

        // divide by the corresponding diagonal A_jj that was skipped in summation
        float a_jj = chunk.matrix_a[j + j * chunk.cols + chunk.id * chunk.rows];

        DLOG("Diag: %f, ", a_jj);

        // if you ensure diagonals are set to 1 if not active this check can be skipped
        if (a_jj == 0.f)
        {
            a_jj = 1.0f;
        }

        x /= a_jj;

        DLOG("X: %f\n", x);

        // update x_j to the new estimate
        chunk.curr_x[j] = x;
    }
}
