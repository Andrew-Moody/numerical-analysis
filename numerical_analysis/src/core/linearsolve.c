#include "linearsolve.h"

#include <stdio.h>
#include <omp.h>

#include "frame.h"

#define PRINT_DEBUG 0

#if PRINT_DEBUG
#define DLOG(...) (printf(__VA_ARGS__))
#else
#define DLOG(...)
#endif


// Solve the equation set using the Jacobi iterative method
void solve_jacobi_single(struct EquationSet eqset, float* residuals, int iterations)
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

    const float* matrix_a = eqset.stiff_bc.elements;
    const float* vector_b = eqset.forces.elements;
    float* vec_x_curr = eqset.displacements.elements;
    const int rows = eqset.stiff_bc.rows;
    const int cols = eqset.stiff_bc.cols;

    // Need space to store the values from the previous iteration
    float* vec_x_prev = malloc(sizeof(*vec_x_prev) * cols);

    // Set initial guess to x_i = b_i / A_ii
    // Convergence may be faster with better initial guesses
    for (int i = 0; i < cols; ++i)
    {
        vec_x_prev[i] = vector_b[i] / matrix_a[i + i * cols];
    }


    for (int t = 0; t < iterations; ++t)
    {
        float sum_sqr_residual = 0;

        for (int j = 0; j < rows; ++j)
        {
            // Multiply the jth row of A times the previous x vector
            // Skip the i != j check and subtract A_ii * x_i afterwards
            // since we will need the full sum anyway for the residual
            float sum_ax = 0;
            for (int i = 0; i < cols; ++i)
            {
                sum_ax += matrix_a[i + j * cols] * vec_x_prev[i];
            }

            DLOG("Sum: %f, ", sum_ax);

            // Subtract the sum from b_j to get the jth residual
            float residual = vector_b[j] - sum_ax;

            DLOG("Residual: %f, ", residual);

            // Add the square residual to the running sum of square residuals
            sum_sqr_residual += residual * residual;

            // Get the diagonal term for the current row
            // Normally would need to check if it could be zero before dividing but
            // that should have been ensured when applying boundary conditions
            float a_jj = matrix_a[j + j * cols];

            DLOG("Diag: %f, ", a_jj);

            // Add x_j * a_jj and divide the result by a_jj to solve for the new estimate of x_j
            // r_j = b_j - sum( A_ij * x_i ) for all i  ==>  x_j = ( b_j - sum (A_ij * x_i)) / A_jj  for all i != j
            float x_j = (residual + a_jj * vec_x_prev[j]) / a_jj;

            DLOG("X: %f\n", x_j);

            // update x_j to the new estimate
            vec_x_curr[j] = x_j;
        }

        // Update previous x to current x for the next iteration if there are still iterations to do
        if (t < iterations - 1)
        {
            for (int i = 0; i < cols; ++i)
            {
                vec_x_prev[i] = vec_x_curr[i];
            }
        }

        // store the norm of residuals for this iteration
        residuals[t] = sqrt(sum_sqr_residual);
    }

    free(vec_x_prev);
}


void solve_jacobi_parallel(struct EquationSet eqset, float* residuals, int iterations, int desired_threads)
{
    // See solve_jacobi_single for more details on the math involved

    if (desired_threads < 1)
    {
        // Most likely not intended
        printf("Warning: Attempted to solve with desired_threads less than 1");
        return;
    }
    else if (desired_threads == 1)
    {
        // Use the single threaded solution instead
        solve_jacobi_single(eqset, residuals, iterations);
        return;
    }

    const float* matrix_a = eqset.stiff_bc.elements;
    const float* vector_b = eqset.forces.elements;
    float* vec_x_curr = eqset.displacements.elements;
    const int rows = eqset.stiff_bc.rows;
    const int cols = eqset.stiff_bc.cols;

    // Need space to store the values from the previous iteration
    float* vec_x_prev = malloc(sizeof(*vec_x_prev) * cols);

    // Multi-threaded
#pragma omp parallel num_threads(desired_threads)
    // for (int t = 0; t < 10; ++t) dohh
    for (int t = 0; t < iterations; ++t)
    {
        float sum_sqr_residual = 0;

        // update multiple rows at a time
#pragma omp for schedule(dynamic)
        for (int j = 0; j < rows; ++j)
        {
            // sum up A_ij * x_i
            float sum_ax = 0;
            for (int i = 0; i < cols; ++i)
            {
                sum_ax += matrix_a[i + j * cols] * vec_x_prev[i];
            }

            // subtract the sum from b_j to get the residual
            float residual = vector_b[j] - sum_ax;

            // Add the square residual to the running sum of square residuals
            sum_sqr_residual += residual * residual;

            // The diagonal element for row j
            float k_jj = matrix_a[j + j * cols];

            // Solve for the new estimate of x_j
            float x_j = (residual + k_jj * vec_x_prev[j]) / k_jj;

            // update x_j to the new estimate
            vec_x_curr[j] = x_j;
        }

        // Update previous x to current x for the next iteration if there are still iterations to do
        if (t < iterations - 1)
        {
            for (int i = 0; i < cols; ++i)
            {
                vec_x_prev[i] = vec_x_curr[i];
            }
        }

        // store the norm of residuals for this iteration
        residuals[t] = sqrt(sum_sqr_residual);
    }

    free(vec_x_prev);
}


void solve_sor_single(struct EquationSet eqset, float* residuals, int iterations, int relax_factor)
{
    // Solves a linear system of the form Ax = b for x using Successive Over-relaxation
    // it is similar to Jacobi method except it uses a weighted blend between the previous x values
    // and the current x values mid iteration

    // if the relaxation factor is equal to 1 then only the current values are used making the method
    // equivalent to the Gauss-Seidel method

    // The relaxtion factor can be fine tuned to improve convergence rate. Both SOR and Gauss-Seidel
    // are easier to gaurantee convergence then Jacobi.

    // The relaxtion factor should typicaly be in the range 0 < rf < 2
    // less or equal to zero means the solution never converges because you aren't updating at all
    // greater or equal to 2 violates convergence gaurantees for symmetric positive definite matrices

    const float* matrix_a = eqset.stiff_bc.elements;
    const float* vector_b = eqset.forces.elements;
    float* vec_x_curr = eqset.displacements.elements;
    const int rows = eqset.stiff_bc.rows;
    const int cols = eqset.stiff_bc.cols;

    // Need space to store the values from the previous iteration
    float* vec_x_prev = malloc(sizeof(*vec_x_prev) * cols);

    // Set initial guess to x_i = b_i / A_ii
    // Convergence may be faster with better initial guesses
    for (int i = 0; i < cols; ++i)
    {
        vec_x_prev[i] = vector_b[i] / matrix_a[i + i * cols];
    }


    for (int t = 0; t < iterations; ++t)
    {
        float sum_sqr_residual = 0;

        for (int j = 0; j < rows; ++j)
        {
            // Multiply the jth row of A times the current x vector
            // Skip the i != j check and subtract A_ii * x_i afterwards
            // since we will need the full sum anyway for the residual
            // not all values will have been updated yet but values x_0 to x_j-1
            // will have. This leads to faster convergence since you are using
            // the new information as soon as you have it
            float sum_ax = 0;
            for (int i = 0; i < cols; ++i)
            {
                sum_ax += matrix_a[i + j * cols] * vec_x_curr[i];
            }

            // Subtract the sum from b_j to get the jth residual
            float residual = vector_b[j] - sum_ax;

            // Add the square residual to the running sum of square residuals
            sum_sqr_residual += residual * residual;

            // Get the diagonal term for the current row
            // Normally would need to check if it could be zero before dividing but
            // that should have been ensured when applying boundary conditions
            float a_jj = matrix_a[j + j * cols];

            // Add x_j * a_jj and divide the result by a_jj to solve for the new estimate of x_j
            // r_j = b_j - sum( A_ij * x_i ) for all i  ==>  x_j = ( b_j - sum (A_ij * x_i)) / A_jj  for all i != j
            float x_j = (residual + a_jj * vec_x_prev[j]) / a_jj;

            // update current x blending between the new (Gauss-Seidel) estimate and previous estimate
            vec_x_curr[j] = relax_factor * x_j + (1 - relax_factor) * vec_x_prev[j];
        }

        // Update previous x to current x for the next iteration if there are still iterations to do
        if (t < iterations - 1)
        {
            for (int i = 0; i < cols; ++i)
            {
                vec_x_prev[i] = vec_x_curr[i];
            }
        }

        // store the norm of residuals for this iteration
        residuals[t] = sqrt(sum_sqr_residual);
    }

    free(vec_x_prev);
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

        DLOG("Offset: %d, ", chunk.id * chunk.rows);

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
