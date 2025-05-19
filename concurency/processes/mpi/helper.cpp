#include <mpi.h>
#include <string>
#include <array>
#include <cstdio>
#include <memory>
#include "mpi.hpp"


int main(int argc, char* argv[]) {
    MPI_Comm parent_comm;
    int rank;
    
    
    // Initialize MPI
    MPI_Init(&argc, &argv);
    
    // Get the parent communicator
    MPI_Comm_get_parent(&parent_comm);
    if (parent_comm == MPI_COMM_NULL) {
        std::cout << std::format("Error: This program should be spawned by another MPI program\n");
        MPI_Finalize();
        return 1;
    }
    
    MPI_Comm_rank(parent_comm, &rank);
    
    // Get the directory path from arguments
    std::string dir_path = ".";
    if (argc > 1) {
        dir_path = argv[1];
    }
    
    // Execute ls command on the directory
    std::string command = "ls -la " + dir_path;
    std::string output = exec_command(command.c_str());
    
    // Send the output back to parent
    MPI_Send(output.data(), output.size(), MPI_CHAR, 0, 0, parent_comm);
    
    std::cout << std::format("Helper sent directory listing to parent\n");
    
    MPI_Finalize();
    return 0;
}