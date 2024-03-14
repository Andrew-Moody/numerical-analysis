#pragma once

#include <stdio.h>

#ifndef ENABLE_MPI
#define ENABLE_MPI 0
#endif

#if ENABLE_MPI
#include <mpi.h>



void send_broadcast(int procs)
{
    MPI_Datatype datatype = MPI_CHAR;
    int datasize;
    MPI_Type_size(datatype, &datasize);
    int tag = 0;
    MPI_Status status = {};

    int count = 256;

    char* sendbuffer = malloc(sizeof(char) * count);

    for (int rank = 0; rank < procs; ++rank)
    {
        sprintf(sendbuffer, "Hello %d, from 0\n", rank);

        int errcode = MPI_Send(sendbuffer, count, datatype, rank, tag, MPI_COMM_WORLD);
        if (errcode != MPI_SUCCESS)
        {
            printf("MPI Error: MPI_Send returned error: %d from rank: 0 to rank: %d\n", errcode, rank);
            return;
        }
    }


    free(sendbuffer);
}

void wait_broadcast(int rank, int procs)
{
    if (rank == 0)
    {
        return;
    }

    MPI_Datatype datatype = MPI_CHAR;
    int datasize;
    MPI_Type_size(datatype, &datasize);
    int tag = 0;
    int source = 0;
    MPI_Status status = {};

    int count = 256;

    char* recvbuffer = malloc(sizeof(char) * count);

    int errcode = MPI_Recv(recvbuffer, count, datatype, source, tag, MPI_COMM_WORLD, &status);
    if (errcode != MPI_SUCCESS)
    {
        printf("MPI Error: MPI_Recv returned error: %d for rank: %d from rank: %d\n", errcode, rank, source);
        return;
    }

    printf("Rank %d: Received message: %s\n", rank, recvbuffer);

    free(recvbuffer);
}

void hot_potato(int rank, int procs)
{
    int next = (rank + 1) % procs;
    int prev = rank == 0 ? procs - 1 : rank - 1;


    MPI_Datatype datatype = MPI_CHAR;
    int datasize;
    MPI_Type_size(datatype, &datasize);
    int tag = 0;
    MPI_Status status = {};

    int count = 256;

    char* sendbuffer = malloc(sizeof(char) * count);
    char* recvbuffer = malloc(sizeof(char) * count);

    sprintf(sendbuffer, "Hello, world! I'm rank %d\n", rank);

    if (rank % 2 == 0)
    {
        // Send first

        int errcode = MPI_Send(sendbuffer, count, datatype, next, tag, MPI_COMM_WORLD);
        if (errcode != MPI_SUCCESS)
        {
            printf("MPI Error: MPI_Send returned error: %d for rank: %d to rank: %d\n", errcode, rank, next);
            return;
        }

        printf("Rank %d: Sent message: %s\n", rank, sendbuffer);

        errcode = MPI_Recv(recvbuffer, count, datatype, prev, tag, MPI_COMM_WORLD, &status);
        if (errcode != MPI_SUCCESS)
        {
            printf("MPI Error: MPI_Recv returned error: %d for rank: %d from rank: %d\n", errcode, rank, prev);
            return;
        }

        //printf("%d: recieved status: source: %d, tag: %d, error: %d\n", rank, status.MPI_SOURCE, status.MPI_TAG, status.MPI_ERROR);

        if (status.MPI_ERROR != MPI_SUCCESS)
        {
            printf("MPI Error: MPI_Recv status.MPI_ERROR was not success: %d for rank: %d from rank: %d\n", status.MPI_ERROR, rank, prev);
            return;
        }

        printf("Rank %d: Received message: %s\n", rank, recvbuffer);

    }
    else
    {
        // recieve first

        int errcode = MPI_Recv(recvbuffer, count, datatype, prev, tag, MPI_COMM_WORLD, &status);
        if (errcode != MPI_SUCCESS)
        {
            printf("MPI Error: MPI_Recv returned error: %d for rank: %d from rank: %d\n", errcode, rank, prev);
            return;
        }

        //printf("%d: recieved status: source: %d, tag: %d, error: %d\n", rank, status.MPI_SOURCE, status.MPI_TAG, status.MPI_ERROR);

        if (status.MPI_ERROR != MPI_SUCCESS)
        {
            printf("MPI Error: MPI_Recv status.MPI_ERROR was not success: %d for rank: %d from rank: %d\n", status.MPI_ERROR, rank, prev);
            return;
        }

        printf("Rank %d: Received message: %s\n", rank, recvbuffer);


        errcode = MPI_Send(sendbuffer, count, datatype, next, tag, MPI_COMM_WORLD);
        if (errcode != MPI_SUCCESS)
        {
            printf("MPI Error: MPI_Send returned error: %d for rank: %d to rank: %d\n", errcode, rank, next);
            return;
        }

        printf("Rank %d: Sent message: %s\n", rank, sendbuffer);
    }

    free(sendbuffer);
    free(recvbuffer);
}

void mpi_test(int* argc, char*** argv)
{
#if ENABLE_MPI

    MPI_Init(argc, argv);

    int procs;
    int rank;
    // Total processes
    MPI_Comm_size(MPI_COMM_WORLD, &procs);

    // Local process index
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    printf("Initialized process %d out of %d\n", rank, procs);

    hot_potato(rank, procs);

    // Lowest level API primitives
    //int MPI_Send(void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
    //int MPI_Recv(void* buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status* status)

    // void* buff is a generic pointer to data to be sent or recieved with count being the number of some primitive
    // element not the size in bytes. The primitive element size is defined by its datatype i.e. MPI_CHAR, MPI_INT
    // This allows code to be less dependent on the sizes of primitive types that may vary from machine to machine

    // dest (and source) is the "rank" of the process, associated with the communicator comm, to which we want to send the buffer
    // tag can be used to help distinguish different types of messages and selectively filter messages

    // When you use send on a source process you have to have a corresponding recieve on the destination process
    // status gives information about the success or failure. It provides a source, tag, and error code

    // There is also MPI_Sendrecv which does send and receive simultaneously

    MPI_Finalize();

    if (rank == 0)
    {
        exit(0);
    }

#else

    printf("MPI is not enabled\n");

#endif
}

#endif
