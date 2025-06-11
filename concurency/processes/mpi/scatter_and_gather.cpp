#include <iostream>
#include <format>
#include <vector>
#include <mpi.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Each process will get SUBSIZES elements
    const int SUBSIZES = 3;
    // The total size of data at root (world_size * SUBSIZES)
    const int TOTAL_SIZE = world_size * SUBSIZES;
    
    std::vector<int> send_data;
    // Buffer to receive scattered data
    std::vector<int> recv_data(SUBSIZES);
    // Buffer to gather results
    std::vector<int> gather_data;
    
    if (world_rank == 0) {
        send_data.resize(TOTAL_SIZE);
        gather_data.resize(TOTAL_SIZE);
        
        // Initialize data
        for (int i = 0; i < TOTAL_SIZE; i++) {
            send_data[i] = i;
        }
        
        std::cout << std::format("Process 0 scattering data:\n");
        for (int i = 0; i < world_size; i++) {
            std::cout << std::format("Process {} will receive: ", i);
            for (int j = 0; j < SUBSIZES; j++) {
                std::cout << std::format("{} ", send_data[i * SUBSIZES + j]);
            }
            std::cout << "\n";
        }
    }
    
    // Scatter the data from process 0 to all processes
    MPI_Scatter(send_data.data(), SUBSIZES, MPI_INT,
                recv_data.data(), SUBSIZES, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Each process modifies its portion
    std::cout << std::format("Process {} received: ", world_rank);
    for (int i = 0; i < SUBSIZES; i++) {
        std::cout << std::format("{} ", recv_data[i]);
        // Square the values
        recv_data[i] = recv_data[i] * recv_data[i];
    }
    std::cout << "\n";
    
    // Gather the modified data back to the root process
    MPI_Gather(recv_data.data(), SUBSIZES, MPI_INT,
              gather_data.data(), SUBSIZES, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Root process prints the gathered data
    if (world_rank == 0) {
        std::cout << std::format("Process 0 gathered:\n");
        for (int i = 0; i < world_size; i++) {
            std::cout << std::format("From process {}: ", i);
            for (int j = 0; j < SUBSIZES; j++) {
                std::cout << std::format("{} ", gather_data[i * SUBSIZES + j]);
            }
            std::cout << "\n";
        }
    }

    MPI_Finalize();
    return 0;
}