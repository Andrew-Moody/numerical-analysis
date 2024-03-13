#pragma once

#include <stdio.h>

#ifndef ENABLE_MPI
#define ENABLE_MPI 0
#endif

#if ENABLE_MPI
#include <mpi.h>
#endif


void mpi_test(int* argc, char*** argv)
{
#if ENABLE_MPI

    MPI_Init(argc, argv);

    int size;
    int rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    printf("Size: %i, Rank: %i\n", size, rank);

    MPI_Finalize();

#else

    printf("MPI is not enabled\n");

#endif
}