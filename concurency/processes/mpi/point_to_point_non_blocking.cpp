#include <iostream>
#include <format>
#include <vector>
#include <mpi.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_size < 2) {
        std::cout << "This example requires at least 2 processes\n";
        MPI_Finalize();
        return 1;
    }

    constexpr int MAX_NUMBERS = 100;
    std::vector<int> numbers(MAX_NUMBERS);
    MPI_Request request;
    MPI_Status status;

    if (world_rank == 0) {
        // Fill the array with data
        for (int i = 0; i < MAX_NUMBERS; i++) {
            numbers[i] = i;
        }
        
        // Non-blocking send
        MPI_Isend(numbers.data(), MAX_NUMBERS, MPI_INT, 1, 0, MPI_COMM_WORLD, &request);
        std::cout << std::format("Process 0 initiated non-blocking send\n");
        
        // Do other work while communication is in progress...
        
        // Wait for send to complete
        MPI_Wait(&request, &status);
        std::cout << std::format("Process 0 completed sending\n");
    } else if (world_rank == 1) {
        // Non-blocking receive
        MPI_Irecv(numbers.data(), MAX_NUMBERS, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
        std::cout << std::format("Process 1 initiated non-blocking receive\n");
        
        // Do other work while waiting for data...
        
        // Wait for receive to complete
        MPI_Wait(&request, &status);
        std::cout << std::format("Process 1 received data. Last number: {}\n", 
                               numbers[MAX_NUMBERS - 1]);
    }

    MPI_Finalize();
    return 0;
}