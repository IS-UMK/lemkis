#pragma once

#include <mpi.h>
#include <array>
#include <memory>

#include <string>
#include <string_view>
#include <vector>
#include <span>
#include <expected>
#include <format>
#include <iostream>
// #include "print.hpp"

/**
 * Execute a shell command and return its output as a string
 */
std::string exec_command(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        return "Error executing command";
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    
    return result;
}


/**
 * Spawn new MPI processes
 * 
 * @param command The executable to run
 * @param args Vector of arguments for the new process
 * @param num_processes Number of processes to spawn
 * @param parent_comm Parent communicator
 * @return New intercommunicator on success or error code
 */
std::expected<MPI_Comm, int> spawn_process(
    std::string_view command,
    const std::vector<std::string>& args,
    int num_processes,
    MPI_Comm parent_comm
) {
    // Convert std::vector<std::string> to char** for MPI
    std::vector<char*> c_args;
    c_args.reserve(args.size() + 1);
    
    for (const auto& arg : args) {
        c_args.push_back(const_cast<char*>(arg.c_str()));
    }
    c_args.push_back(nullptr); // NULL-terminated
    
    // Prepare for spawn
    std::vector<int> error_codes(num_processes);
    int rank;
    MPI_Comm new_intercomm;
    
    // Get current rank
    MPI_Comm_rank(parent_comm, &rank);
    
    std::cout << std::format("Process {} spawning {} new processes running {}\n", 
              rank, num_processes, command);
    
    // Spawn the new processes
    int result = MPI_Comm_spawn(
        command.data(),    // Program to execute
        c_args.data(),     // Arguments for the program
        num_processes,     // Number of processes to spawn
        MPI_INFO_NULL,     // No special info
        0,                 // Root process (who spawns)
        parent_comm,       // Parent communicator
        &new_intercomm,    // New intercommunicator
        error_codes.data() // Error codes for each process
    );
     std::cout << std::format("Process {} spawned {} new processes running {}\n", 
              rank, num_processes, command);
    
    // Check for errors in spawning
    if (result != MPI_SUCCESS) {
        std::cout << std::format("Error spawning processes: {}\n", result);
        return std::unexpected(result);
    }
    
    // Check individual process spawn errors
    for (int i = 0; i < num_processes; i++) {
        if (error_codes[i] != MPI_SUCCESS) {
            std::cout << std::format("Warning: Error spawning process {}: {}\n", i, error_codes[i]);
        }
    }
    
    std::cout << std::format("Process {} successfully spawned new processes\n", rank);
    return new_intercomm;
}
