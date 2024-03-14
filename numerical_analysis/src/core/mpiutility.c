#include "mpiutility.h"

#include <stdlib.h>
#include <stdio.h>

#ifndef ENABLE_MPI
#define ENABLE_MPI 0
#endif

#if ENABLE_MPI
#include <mpi.h>
#endif

// These are known only to this file and are accessed through get_rank and get_proc respectively
int RANK = -1;
int PROCS = -1;

int INITIALIZED = 0;

void initialize_mpi(int* argc, char*** argv)
{
#if ENABLE_MPI

    if (INITIALIZED)
    {
        fprintf(stderr, "Attempted to Initialize MPI multiple times\n");
        return;
    }

    MPI_Init(argc, argv);

    // Total processes
    MPI_Comm_size(MPI_COMM_WORLD, &PROCS);

    // Local process index
    MPI_Comm_rank(MPI_COMM_WORLD, &RANK);

#endif
}

void finalize_mpi(int should_exit)
{
#if ENABLE_MPI

    MPI_Finalize();

    if (should_exit && get_rank_mpi() == 0)
    {
        exit(0);
    }

#endif
}


int get_rank_mpi(void)
{
    if (RANK == -1)
    {
        fprintf(stderr, "Attempted to access local rank but MPI is not initialized\n");
    }

    return RANK;
}

int get_procs_mpi(void)
{
    if (PROCS == -1)
    {
        fprintf(stderr, "Attempted to access total processes but MPI is not initialized\n");
    }

    return PROCS;
}

int get_initialized_mpi(void)
{
    return INITIALIZED;
}