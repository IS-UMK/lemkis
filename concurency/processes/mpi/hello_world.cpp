#include <iostream>
#include <format>
#include <string>
#include <mpi.h>

int main(int argc, char** argv) {
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the rank (ID) of the process and total number of processes
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Print a message from each process
    std::cout << std::format("Hello from process {} of {}\n", world_rank, world_size);

    // Finalize the MPI environment
    MPI_Finalize();
    return EXIT_SUCCESS;  // Use standard constant instead of 0
}