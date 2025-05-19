#include <iostream>
#include <format>
#include <vector>
#include <mpi.h>

// Process ranks
enum ProcessRank {
    SENDER = 0,
    RECEIVER = 1
};

// MPI constants
enum MPIConstants {
    DEFAULT_TAG = 0
};

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Define minimum required processes
    constexpr int MIN_REQUIRED_PROCESSES = 2;
    
    if (world_size < MIN_REQUIRED_PROCESSES) {
        std::cout << "This example requires at least " << MIN_REQUIRED_PROCESSES << " processes\n";
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    constexpr int MAX_NUMBERS = 100;
    std::vector<int> numbers;
    
    if (world_rank == ProcessRank::SENDER) {
        // Process SENDER sends data to process RECEIVER
        numbers.resize(MAX_NUMBERS);
        for (int i = 0; i < MAX_NUMBERS; i++) {
            numbers[i] = i;
        }
        std::cout << std::format("Process SENDER sending data to process RECEIVER\n");
                  
        MPI_Send(numbers.data(), MAX_NUMBERS, MPI_INT, 
                ProcessRank::RECEIVER, MPIConstants::DEFAULT_TAG, MPI_COMM_WORLD);
                
    } else if (world_rank == ProcessRank::RECEIVER) {
        // Process RECEIVER receives data from process SENDER
        numbers.resize(MAX_NUMBERS);
        MPI_Status status;
        
        MPI_Recv(numbers.data(), MAX_NUMBERS, MPI_INT, 
                ProcessRank::SENDER, MPIConstants::DEFAULT_TAG, MPI_COMM_WORLD, &status);
                
        std::cout << std::format("Process RECEIVER received data from process SENDER. Last number: {}\n", 
                              numbers[MAX_NUMBERS - 1]);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}