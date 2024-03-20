#pragma once

struct EquationSet;

int solve_equations_mpi(struct EquationSet* eqset, int iterations);

int send_equations(struct EquationSet* eqset, int dest);

int recv_equations(struct EquationSet* eqset, int src);
