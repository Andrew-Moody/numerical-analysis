#include "fluid.h"

#include <stdlib.h>
#include <stdio.h>

void convection1D(void)
{
    float x_begin = 0.0f;
    float x_end = 2.0f;
    float u_initial = 1.0f;
    float wavespeed = 1.0f;

    int x_steps = 41;
    int t_steps = 25;
    float t_total = 0.6f;
    
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

    // Solve with finite difference
    for (int t_idx = 1; t_idx < t_steps; ++t_idx)
    {
        // Iterate in resverse to allow updating the array
        // in place while preserving the prev value for u[x_idx - 1]
        for (int x_idx = x_steps - 1; x_idx > 0; --x_idx)
        {
            // Linear Convection
            //u[x_idx] = u[x_idx] - (wavespeed * (u[x_idx] - u[x_idx - 1]) * dt / dx);

            // NonLinear Convection (replace constant wavespeed with local velocity)
            u[x_idx] = u[x_idx] - (u[x_idx] * (u[x_idx] - u[x_idx - 1]) * dt / dx);
        }
    }

    for (int i = 0; i < x_steps; ++i)
    {
        printf("%f\n", u[i]);
    }

    free(u);
}
