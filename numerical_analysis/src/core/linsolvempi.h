#pragma once

struct EquationSet;

int solve_equations_mpi(struct EquationSet* eqset, int iterations);

int send_equations(struct EquationSet* eqset, int dest);

int recv_equations(struct EquationSet* eqset, int src);

void update_jacobi(float* matrix_a, float* prev_x, float* curr_x, float* vector_b, int rows, int cols);