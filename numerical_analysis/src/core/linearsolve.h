#pragma once

struct EquationSet;

void solve_jacobi_omp(struct EquationSet* eqset, int iterations, int desired_threads);