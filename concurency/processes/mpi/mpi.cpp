#include <mpi.h>
#include <iostream>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <span>
#include <expected>
#include <format>
#include "mpi.hpp"

// int main(int argc, char* argv[]) {
//     // Initialize the MPI environment
//     MPI_Init(&argc, &argv);

//     // Get the number of processes
//     int world_size;
//     MPI_Comm_size(MPI_COMM_WORLD, &world_size);

//     // Get the rank of the process
//     int world_rank;
//     MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

//     // Print a hello message from each process
//     std::cout << "Hello from processor " << world_rank 
//               << " out of " << world_size << " processors." << std::endl;


//     MPI_Comm parent_comm;
//     int rank;
    
    
//     // Check if this process was spawned
//     MPI_Comm_get_parent(&parent_comm);
    
//     if (parent_comm == MPI_COMM_NULL) {
//         // This is the parent process
//         MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//         std::cout << std::format("Parent process {} started\n", rank);
        
//         // Get current source directory path
//         auto source_path = std::filesystem::current_path();
//         std::cout << std::format("Listing contents of: {}\n", source_path.string());
        
//         // Prepare arguments for the child process
//         std::vector<std::string> args = {source_path.string()};
        
//         // Spawn a child process
//         auto result = spawn_process(std::filesystem::absolute("./build/debug/concurency/processes/mpi/helper").string(), args, 1, MPI_COMM_WORLD);
        
//         if (!result) {
//             std::cout << std::format("Failed to spawn child process: error {}\n", result.error());
//             MPI_Finalize();
//             return 1;
//         }
        
//         MPI_Comm child_comm = *result;
        
//         // Receive directory listing from child
//         MPI_Status status;
//         int count;
//         MPI_Probe(0, 0, child_comm, &status);
//         MPI_Get_count(&status, MPI_CHAR, &count);
        
//         std::vector<char> buffer(count);
//         MPI_Recv(buffer.data(), count, MPI_CHAR, 0, 0, child_comm, &status);
        
//         // Output the directory listing
//         std::string listing(buffer.data(), buffer.size());
//         std::cout << std::format("Directory contents:\n{}", listing);
//     } 
//     else {
//         // This is the child helper process
//         MPI_Comm_rank(parent_comm, &rank);
        
//         // Get the directory path from arguments
//         std::string dir_path = ".";
//         if (argc > 1) {
//             dir_path = argv[1];
//         }
        
//         // Execute ls command on the directory
//         std::string command = "ls -la " + dir_path;
//         std::string output = exec_command(command.c_str());
        
//         // Send the output back to parent
//         MPI_Send(output.data(), output.size(), MPI_CHAR, 0, 0, parent_comm);
        
//         std::cout << std::format("Child sent directory listing to parent\n");
//     }
    
//     // Finalize the MPI environment
//     MPI_Finalize();

//     return 0;
// }


int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        
    
    std::cout << std::format("Process {} started\n", rank)<< std::endl;;
    
    if (rank == 0) {
        std::cout << std::format("Directory contents:\n{}\n", exec_command("ls -la"))<< std::endl;;
    } else {
        std::cout << std::format("hellow from process with rank: {}\n", rank);
    }
    MPI_Finalize();
    return 0;
}