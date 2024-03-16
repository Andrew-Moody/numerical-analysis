#pragma once

// Call this first in main since it may modify command line arguments
void initialize_mpi(int* argc, char*** argv);

// Call before returning from main
void finalize_mpi(int should_exit);

// Get the current process rank
int get_rank_mpi(void);

// Get the current number of processes
int get_procs_mpi(void);

// Get the rank associated with the "main" process
int get_main_mpi(void);

// Check if MPI has been initialized
int get_initialized_mpi(void);