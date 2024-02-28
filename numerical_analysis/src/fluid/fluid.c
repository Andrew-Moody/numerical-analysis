#include "fluid.h"

#include <stdlib.h>
#include <stdio.h>

#include <omp.h>

void finiteDifference1D(void)
{
    float x_begin = 0.0f;
    float x_end = 2.0f;
    float u_initial = 1.0f;
    float wavespeed = 1.0f;
    float viscosity = 0.3f;

    int x_steps = 41;
    int t_steps = 25;
    float t_total = 0.6f;

    //int t_steps = 20;
    //float t_total = 0.0316666f;
    
    float dx = (x_end - x_begin) / (float)(x_steps - 1);
    float dt = t_total / (float)(t_steps - 1);

    float* u = (float*)malloc(x_steps * sizeof(float));

    
    // Setup initial conditions
    for (int i = 0; i < x_steps; ++i)
    {
        // Step up at x = 0.5 and step down at x = 1.0
        float x = x_begin + i * dx;
        if (x >= 0.5f && x <= 1.0f)
        {
            u[i] = 1.0f + u_initial;
        }
        else
        {
            u[i] = 1.0f;
        }
    }

    // Advance through time updating u(x, t) at each step via finite difference (forward difference for time)
    for (int t_idx = 1; t_idx < t_steps; ++t_idx)
    {
        float u_xprev = u[0]; // used in place of u[x_idx - 1] since it will be overwritten

        // values at i = 0 and i = x_steps - 1 are constrained by boundary conditions
        for (int x_idx = 1; x_idx < x_steps - 1; ++x_idx)
        {
            // save the current value before overwriting
            float u_xcurr = u[x_idx];

            // Linear Convection (example of first order derivative solved with backward difference)
            u[x_idx] = u_xcurr - (wavespeed * (u_xcurr - u_xprev) * dt / dx);

            // NonLinear Convection (replace constant wavespeed with local velocity)
            //u[x_idx] = u_xcurr - (u_xcurr * (u_xcurr - u_xprev) * dt / dx);

            // Diffusion (example of second order derivative solved with central difference)
            //u[x_idx] = u_xcurr + (viscosity * (u[x_idx + 1] - 2.0f * u_xcurr + u_xprev) * dt / (dx * dx));

            // Burgers' Equation (combined diffusion and nonlinear convection)
            //u[x_idx] = u_xcurr  - (u_xcurr * (u_xcurr - u_xprev) * dt / dx)
            //    + (viscosity * (u[x_idx + 1] - 2.0f * u_xcurr + u_xprev) * dt / (dx * dx));

            // current value will be the previous value for the next element
            u_xprev = u_xcurr;
        }
    }

    for (int i = 0; i < x_steps; ++i)
    {
        printf("%f, %f\n", i * dx, u[i]);
    }

    free(u);
}

void finiteDifference2D(void)
{
    const int desired_threads = 8;

    float x_begin = 0.0f;
    float x_end = 2.0f;
    float y_begin = 0.0f;
    float y_end = 2.0f;
    float vel_min= 1.0f;
    float vel_max = 2.0f;
    float viscosity = 0.01f;
    float sigma = 0.2f;

    int x_steps = 801;
    int y_steps = 801;
    int t_steps = 1200;
    
    float dx = (x_end - x_begin) / (float)(x_steps - 1);
    float dy = (y_end - y_begin) / (float)(y_steps - 1);
    float dt = sigma * dx * dy / viscosity;


    // velocity fields
    float* u = (float*)malloc(x_steps * y_steps * sizeof(float));
    float* v = (float*)malloc(x_steps * y_steps * sizeof(float));
    
    // Setup initial conditions
    // u(x, y) = 2 for x and y between 0.5 and 1 otherwise u = 1
    for(int j = 0; j < y_steps; ++j)
    {
        for (int i = 0; i < x_steps; ++i)
        {
            int index = i + j * x_steps;

            float x = x_begin + i * dx;
            float y = y_begin + j * dy;
            if (x >= 0.5f && x <= 1.0f && y >= 0.5f && y <= 1.0f)
            {
                u[index] = vel_max;
                v[index] = vel_max;
            }
            else
            {
                u[index] = vel_min;
                v[index] = vel_min;
            }
        }
    }

    // The process of updating each timestep requires values from the previous timestep
    // that are overwritten so they must be cached. If it is okay to double the memory 
    // usage, simply allocating 2 sets of arrays and swapping between them for each timestep
    // works fine and plays nice with the parallel for construct. here only one additional row 
    // will be allocated per thread but this requires exact control over the way chunks are
    // split to set initial values for each timestep

    // used to cache the values of a row for use when updating the next row
    float* u_rows_prev = NULL;
    float* v_rows_prev = NULL;

    // Advance through time updating u(x, t) at each step via finite difference (forward difference for time)
    #pragma omp parallel shared(u, v, u_rows_prev, v_rows_prev) num_threads(desired_threads)
    {
        // Prefer to wait till the actual num of launched threads is known rather than use desired_threads
        // also simplifies handling case when -fopenmp compiler flag is not used
        const int threads = omp_get_num_threads();
        const int thread_idx = omp_get_thread_num();

        // Each thread needs its own cache row but wanted to try allocating them all at once
        #pragma omp single
        {
            u_rows_prev = (float*)malloc(x_steps * threads * sizeof(float));
            v_rows_prev = (float*)malloc(x_steps * threads * sizeof(float));
        }

        // offset for each thread into the set of cache rows
        float* u_row_prev = u_rows_prev + thread_idx * x_steps;
        float* v_row_prev = v_rows_prev + thread_idx * x_steps;

        // manually control loop splitting
        // split into chunks by rounding up the division by number of threads
        // the last thread may do up to threads-1 less iterations than other threads
        // every thread does one additional iteration rather than the first thread doing 
        // up to threads-1 iterations after all other threads have finished
        // when iterations is large vs threads the difference is small either way
        const int chunk_rows = 1 + (y_steps - 2) / threads;
        const int chunk_rows_last = y_steps - 2 - chunk_rows * (threads - 1);

        const int y_begin = thread_idx * chunk_rows + 1; // skip first row
        const int y_end = y_begin + (thread_idx != threads - 1 ? chunk_rows : chunk_rows_last); // last chunk has fewer rows

        //printf("Thread: %d, rows: %d, rows_last: %d, y_begin: %d, y_end: %d\n", thread_idx, chunk_rows, chunk_rows_last, y_begin, y_end);

        // Update the velocity field for each timestep
        for (int t_idx = 1; t_idx < t_steps; ++t_idx)
        {
            // reset the cache rows for the next timestep to the row before chunk start
            for (int x_idx = 0; x_idx < x_steps; ++x_idx)
            {
                //int index = x_idx
                u_row_prev[x_idx] = u[x_idx + (y_begin - 1) * x_steps];
                v_row_prev[x_idx] = v[x_idx + (y_begin - 1) * x_steps];
            }

            // Do not start updating until all threads have set their row cache since that reads
            // values outside the current threads chunk
            #pragma omp barrier

            // values at i,j = 0 and i,j = x_steps - 1,y_steps - 1 are constrained by boundary conditions
            // so we need only iterate from i,j = 1 to i,j = x_steps - 2,y_steps - 2
            for (int y_idx = y_begin; y_idx < y_end; ++y_idx)
            {
                for (int x_idx = 1; x_idx < x_steps - 1; ++x_idx)
                {
                    int vel_idx = x_idx + y_idx * x_steps;

                    // save the current value before overwriting
                    float u_curr = u[vel_idx];
                    float u_xprev = u_row_prev[x_idx - 1];
                    float u_yprev = u_row_prev[x_idx];
                    float u_xnext = u[vel_idx + 1];
                    float u_ynext = u[vel_idx + x_steps];

                    float v_curr = v[vel_idx];
                    float v_xprev = v_row_prev[x_idx - 1];
                    float v_yprev = v_row_prev[x_idx];
                    float v_xnext = u[vel_idx + 1];
                    float v_ynext = u[vel_idx + x_steps];

                    // Linear Convection (example of first order derivative solved with backward difference)
                    //float wavespeed = 1.0f;
                    //u[vel_idx] = u_curr - (wavespeed * (u_curr - u_xprev) * dt / dx) - (wavespeed * (u_curr - u_yprev) * dt / dy);

                    // NonLinear Convection (replace constant wavespeed with local velocity)
                    //u[vel_idx] = u_curr - (u_curr * (u_curr - u_xprev) * dt / dx) - (v_curr * (u_curr - u_yprev) * dt / dy);
                    //v[vel_idx] = v_curr - (u_curr * (v_curr - v_xprev) * dt / dx) - (v_curr * (v_curr - v_yprev) * dt / dy);

                    // Diffusion (example of second order derivative solved with central difference)
                    /* u[vel_idx] = u_curr + (viscosity * (u_xnext - 2.0f * u_curr + u_xprev) * dt / (dx * dx))
                                        + (viscosity * (u_ynext - 2.0f * u_curr + u_yprev) * dt / (dy * dy));

                    v[vel_idx] = v_curr + (viscosity * (v_xnext - 2.0f * v_curr + v_xprev) * dt / (dx * dx))
                                        + (viscosity * (v_ynext - 2.0f * v_curr + v_yprev) * dt / (dy * dy)); */

                    // Burgers' Equation (combined diffusion and nonlinear convection)
                    u[vel_idx] = u_curr + (viscosity * (u_xnext - 2.0f * u_curr + u_xprev) * dt / (dx * dx))
                                        + (viscosity * (u_ynext - 2.0f * u_curr + u_yprev) * dt / (dy * dy))
                                        - (u_curr * (u_curr - u_xprev) * dt / dx) - (v_curr * (u_curr - u_yprev) * dt / dy);

                    v[vel_idx] = v_curr + (viscosity * (v_xnext - 2.0f * v_curr + v_xprev) * dt / (dx * dx))
                                        + (viscosity * (v_ynext - 2.0f * v_curr + v_yprev) * dt / (dy * dy))
                                        - (u_curr * (v_curr - v_xprev) * dt / dx) - (v_curr * (v_curr - v_yprev) * dt / dy);

                    // current value will be the previous value for the next element
                    u_row_prev[x_idx] = u_curr;
                    v_row_prev[x_idx] = v_curr;
                }
            }

            // Do not move to the next timestep until all threads have finished the current timestep
            #pragma omp barrier
        }
    }
    
    // Save the 2d velocity field as a ppm image
    // with color scaled with the velocity (red for u, green for v)
    int width = x_steps;
    int height = y_steps;

    unsigned char* pixels = malloc(width * height * 3);

    for(int j = 0; j < y_steps; ++j)
    {
        for (int i = 0; i < x_steps; ++i)
        {
            int vel_idx = i + j * x_steps;

            // flip y values vertically so positive y points upward in output image
            int p_idx = 3 * (i + (y_steps - 1 - j) * x_steps);

            pixels[p_idx] = (unsigned char)(255.999f * (u[vel_idx] - vel_min) / (vel_max - vel_min));
            pixels[p_idx + 1] = (unsigned char)(255.999f * (v[vel_idx] - vel_min) / (vel_max - vel_min));
            pixels[p_idx + 2] = 0;
        }
    }

    FILE *image = fopen("velocityfield.ppm", "w");

    if (!image)
    {
        printf("failed to open file");
        exit(1);
    }

    fprintf(image, "P6\n%d %d\n255\n",width, height);

    fwrite(pixels, 1, width * height * 3, image);

    fclose(image);

    free(pixels);
    free(v_rows_prev);
    free(u_rows_prev);
    free(v);
    free(u);
}
