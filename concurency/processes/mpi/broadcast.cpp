#include <iostream>
#include <format>
#include <vector>
#include <mpi.h>

// MPI process roles
enum ProcessRole {
    ROOT = 0  // Process that initiates the broadcast
};

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Define array parameters
    constexpr int ARRAY_SIZE = 10;
    constexpr int FIRST_ELEMENT_INDEX = 0;
    constexpr int SECOND_ELEMENT_INDEX = 1;
    constexpr int THIRD_ELEMENT_INDEX = 2;
    constexpr int MULTIPLIER = 100;  // Used to generate sample data
    
    std::vector<int> data(ARRAY_SIZE);
    
    if (world_rank == ProcessRole::ROOT) {
        // ROOT process initializes the data
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] = i * MULTIPLIER;
        }
        std::cout << std::format("Process ROOT broadcasting data: {}, {}, {}...\n",  
                               data[FIRST_ELEMENT_INDEX], 
                               data[SECOND_ELEMENT_INDEX], 
                               data[THIRD_ELEMENT_INDEX]);
    }
    
    // Broadcast data from ROOT process to all processes
    MPI_Bcast(data.data(), ARRAY_SIZE, MPI_INT, ProcessRole::ROOT, MPI_COMM_WORLD);
    
    // Now all processes have the data
    std::cout << std::format("Process {}: After broadcast, first three values: {}, {}, {}\n",
                           world_rank, 
                           data[FIRST_ELEMENT_INDEX], 
                           data[SECOND_ELEMENT_INDEX], 
                           data[THIRD_ELEMENT_INDEX]);

    MPI_Finalize();
    return EXIT_SUCCESS;
}