#include <iostream>
#include <format>
#include <vector>
#include <chrono>
#include <mpi.h>

// Process roles
enum ProcessRole {
    ROOT = 0  // Coordinator process
};

// MPI constants
enum MPIConstants {
    DEFAULT_TAG = 0
};

// Matrix operation constants
struct MatrixConstants {
    static constexpr int DEFAULT_PREVIEW_SIZE = 3;  // Number of rows/columns to display
};

void initialize_matrix(std::vector<double>& matrix, int rows, int cols) {
    constexpr double ROW_FACTOR = 0.1;    // Multiplier for row index
    constexpr double COL_FACTOR = 0.01;   // Multiplier for column index
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i * cols + j] = i * ROW_FACTOR + j * COL_FACTOR;
        }
    }
}

void print_matrix_part(const std::vector<double>& matrix, int rows, int cols, 
                       const std::string& name, int max_print = MatrixConstants::DEFAULT_PREVIEW_SIZE) {
    std::cout << name << " (showing up to " << max_print << "x" << max_print << "):\n";
    int print_rows = std::min(rows, max_print);
    int print_cols = std::min(cols, max_print);
    
    for (int i = 0; i < print_rows; i++) {
        for (int j = 0; j < print_cols; j++) {
            std::cout << std::format("{:.2f} ", matrix[i * cols + j]);
        }
        if (print_cols < cols) std::cout << "...";
        std::cout << "\n";
    }
    if (print_rows < rows) std::cout << "...\n";
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Matrix dimensions
    constexpr int M = 1000;  // A: M x K (rows x cols)
    constexpr int K = 1000;  // B: K x N (rows x cols)
    constexpr int N = 1000;  // C: M x N (rows x cols)
    
    // Root process initializes matrices A and B
    std::vector<double> A, B, C;
    if (world_rank == ProcessRole::ROOT) {
        A.resize(M * K);
        B.resize(K * N);
        C.resize(M * N, 0.0);
        
        initialize_matrix(A, M, K);
        initialize_matrix(B, K, N);
        
        print_matrix_part(A, M, K, "Matrix A");
        print_matrix_part(B, K, N, "Matrix B");
    }
    
    // Broadcast matrix B to all processes
    // (We're dividing matrix A, but all processes need all of matrix B)
    if (world_rank == ProcessRole::ROOT) {
        std::cout << "Broadcasting matrix B to all processes...\n";
    }
    
    B.resize(K * N); // All processes need space for B
    MPI_Bcast(B.data(), K * N, MPI_DOUBLE, ProcessRole::ROOT, MPI_COMM_WORLD);
    
    // Determine how many rows each process gets
    int rows_per_process = M / world_size;
    int leftover_rows = M % world_size;
    
    // Calculate local rows for this process
    int my_rows = (world_rank < leftover_rows) ? 
                  rows_per_process + 1 : rows_per_process;
    
    // Calculate starting row for this process
    int my_start_row = world_rank < leftover_rows ?
                       world_rank * (rows_per_process + 1) :
                       leftover_rows * (rows_per_process + 1) + 
                       (world_rank - leftover_rows) * rows_per_process;
    
    // Allocate local storage for my portion of A and C
    std::vector<double> local_A(my_rows * K);
    std::vector<double> local_C(my_rows * N, 0.0);
    
    // Scatter rows of A to all processes
    if (world_rank == ProcessRole::ROOT) {
        std::cout << "Scattering matrix A to all processes...\n";
        
        // First, send my own portion to local_A
        std::copy(A.begin(), A.begin() + my_rows * K, local_A.begin());
        
        // Send portions to other processes
        for (int proc_id = 1; proc_id < world_size; proc_id++) {
            int rows_for_proc = (proc_id < leftover_rows) ? 
                             rows_per_process + 1 : rows_per_process;
            int row_start_proc = proc_id < leftover_rows ?
                             proc_id * (rows_per_process + 1) :
                             leftover_rows * (rows_per_process + 1) + 
                             (proc_id - leftover_rows) * rows_per_process;
            
            MPI_Send(&A[row_start_proc * K], rows_for_proc * K, MPI_DOUBLE, 
                    proc_id, MPIConstants::DEFAULT_TAG, MPI_COMM_WORLD);
        }
    } else {
        // Receive my portion from process ROOT
        MPI_Status status;
        MPI_Recv(local_A.data(), my_rows * K, MPI_DOUBLE, 
                ProcessRole::ROOT, MPIConstants::DEFAULT_TAG, MPI_COMM_WORLD, &status);
    }
    
    // Perform local matrix multiplication
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < my_rows; i++) {
        for (int j = 0; j < N; j++) {
            double sum = 0.0;
            for (int k = 0; k < K; k++) {
                sum += local_A[i * K + k] * B[k * N + j];
            }
            local_C[i * N + j] = sum;
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    
    std::cout << std::format("Process {}: Computed {} rows in {:.6f} seconds\n", 
                           world_rank, my_rows, elapsed.count());
    
    // Gather results back to process ROOT
    if (world_rank == ProcessRole::ROOT) {
        std::cout << "Gathering results back to process ROOT...\n";
        
        // First, copy my results to C
        std::copy(local_C.begin(), local_C.end(), C.begin());
        
        // Receive results from other processes
        for (int proc_id = 1; proc_id < world_size; proc_id++) {
            int rows_for_proc = (proc_id < leftover_rows) ? 
                             rows_per_process + 1 : rows_per_process;
            int row_start_proc = proc_id < leftover_rows ?
                             proc_id * (rows_per_process + 1) :
                             leftover_rows * (rows_per_process + 1) + 
                             (proc_id - leftover_rows) * rows_per_process;
            
            std::vector<double> temp_C(rows_for_proc * N);
            MPI_Status status;
            MPI_Recv(temp_C.data(), rows_for_proc * N, MPI_DOUBLE, 
                    proc_id, MPIConstants::DEFAULT_TAG, MPI_COMM_WORLD, &status);
            
            // Copy received data to the main result matrix
            std::copy(temp_C.begin(), temp_C.end(), 
                     C.begin() + row_start_proc * N);
        }
        
        print_matrix_part(C, M, N, "Result Matrix C");
    } else {
        // Send results to process ROOT
        MPI_Send(local_C.data(), my_rows * N, MPI_DOUBLE, 
               ProcessRole::ROOT, MPIConstants::DEFAULT_TAG, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}