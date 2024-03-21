#pragma once

struct EquationSet;

// Non owning. Just a view for a full or partial equation set
struct EquationChunk
{
    float* matrix_a;
    float* vector_b;
    float* prev_x;
    float* curr_x;
    int rows;
    int cols;
    int id;
};

// Solve the equation set using the Jacobi iterative method
void solve_jacobi_single(struct EquationSet eqset, float* residuals, int iterations);

// Use OpenMP to solve with the Jacobi method using multiple threads
void solve_jacobi_parallel(struct EquationSet eqset, float* residuals, int iterations, int desired_threads);

// Solve the equation set using Successive Over-relaxation (or Gauss-Seidel if relaxation factor = 1)
void solve_sor_single(struct EquationSet eqset, float* residuals, int iterations, int relax_factor);

// Update a chunk of an equation set for one iteration (used with MPI)
void update_chunk_jacobi(struct EquationChunk chunk);
