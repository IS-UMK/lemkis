#include <iostream>
#include <format>
#include <mpi.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Each process has a value based on its rank
    int local_value = (world_rank + 1) * 10;
    int global_sum = 0;
    
    std::cout << std::format("Process {}: My local value is {}\n", world_rank, local_value);
    
    // Sum all values and store the result in process 0
    MPI_Reduce(&local_value, &global_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (world_rank == 0) {
        std::cout << std::format("Process 0: Global sum = {}\n", global_sum);
    }

    MPI_Finalize();
    return 0;
}