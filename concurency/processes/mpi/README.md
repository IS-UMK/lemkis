# Exercise for today 

Implement matrix multiplication algorithm.


# Configuration

You may need to install ``openmpi openmpi-devel``, e.g.
```bash
sudo apt update
sudo apt install openmpi-bin libopenmpi-dev
```
YOu can install openmpi  manually [version 5](https://www.open-mpi.org/software/ompi/v5.0/) with for example the following configuration:
```bash
CC=clang-19 CXX=clang++-19 ./configure --prefix=$HOME/opt/openmpi --disable-mpi-fortran
```
You may need to update paths
```bash
 export PATH=/usr/lib64/mpi/gcc/openmpi5/bin:$PATH
 export LD_LIBRARY_PATH=/usr/lib64/mpi/gcc/openmpi5/lib64:$LD_LIBRARY_PATH
```

# local building
If you want to build locally (not whole lemkis - currently not working) do this:
create `build` director `mkdir build` go to it `cd build` and execute
```bash
CXX=mpicxx cmake .. && make
```

# running
```bash
mpirun -n <number_of_processes_to_launch> <program>
```


# MPI Tutorial 

## Introduction to MPI

The Message Passing Interface (MPI) is a standardized and portable message-passing library designed to function on parallel computing architectures. MPI is particularly useful for distributed-memory systems where each process has its own memory space.

### What is MPI?

MPI is:
- A **communication protocol** for parallel computing
- A **library specification** for message passing
- The **de facto standard** for distributed-memory parallel programming
- **Portable** across various hardware platforms
- Available with implementations like Open MPI, MPICH, and Intel MPI

### Why Use MPI?

- **Performance**: Efficiently distributes computational workloads across multiple processors
- **Scalability**: Scales from small clusters to supercomputers with thousands of nodes
- **Portability**: The same MPI code can run on different architectures
- **Standardization**: Well-established standard with consistent APIs


## Basic MPI Concepts

### MPI Program Structure

Every MPI program follows this basic structure:

1. **Initialize** the MPI environment
2. Perform **computation and communication**
3. **Finalize** the MPI environment

### Hello World Example

```cpp
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
```

## Point-to-Point Communication

### Sending and Receiving Messages

MPI supports direct communication between processes using `MPI_Send` and `MPI_Recv`.

```cpp
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
        std::cout << std::format("Process {} sending data to process {}\n", 
                  ProcessRank::SENDER, ProcessRank::RECEIVER);
                  
        MPI_Send(numbers.data(), MAX_NUMBERS, MPI_INT, 
                ProcessRank::RECEIVER, MPIConstants::DEFAULT_TAG, MPI_COMM_WORLD);
                
    } else if (world_rank == ProcessRank::RECEIVER) {
        // Process RECEIVER receives data from process SENDER
        numbers.resize(MAX_NUMBERS);
        MPI_Status status;
        
        MPI_Recv(numbers.data(), MAX_NUMBERS, MPI_INT, 
                ProcessRank::SENDER, MPIConstants::DEFAULT_TAG, MPI_COMM_WORLD, &status);
                
        std::cout << std::format("Process {} received data from process {}. Last number: {}\n", 
                              ProcessRank::RECEIVER, ProcessRank::SENDER, 
                              numbers[MAX_NUMBERS - 1]);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
```

### Blocking vs. Non-Blocking Communication

#### Blocking Communication:
- `MPI_Send` and `MPI_Recv`: Block until communication completes
- Simple to use but can cause deadlocks if not careful

#### Non-Blocking Communication:
- `MPI_Isend` and `MPI_Irecv`: Return immediately, allowing computation to continue
- Requires extra steps to check completion

```cpp
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
```

## Collective Communication

Collective operations involve all processes in a communicator.

### Broadcast

Sends data from one process to all others.

```cpp
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
        std::cout << std::format("Process {} broadcasting data: {}, {}, {}...\n", 
                               ProcessRole::ROOT, 
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
```

### Scatter and Gather

Scatter distributes data from one process to all processes. Gather collects data from all processes.

```cpp
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
```

### Reduce

Combines data from all processes using a specific operation and stores the result in one process.

```cpp
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
```

## Advanced MPI Topics

### Custom Data Types

For sending complex data structures:

```cpp
#include <iostream>
#include <format>
#include <vector>
#include <string>
#include <mpi.h>

struct Person {
    int id;
    int age;
    double height;
    char name[30];
};

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

    // Define MPI datatype for Person
    MPI_Datatype person_type;
    int blocks[4] = {1, 1, 1, 30};  // Number of each element
    MPI_Aint displacements[4];
    MPI_Datatype types[4] = {MPI_INT, MPI_INT, MPI_DOUBLE, MPI_CHAR};
    
    Person sample_person;
    
    // Calculate displacements
    MPI_Aint base_address;
    MPI_Get_address(&sample_person, &base_address);
    MPI_Get_address(&sample_person.id, &displacements[0]);
    MPI_Get_address(&sample_person.age, &displacements[1]);
    MPI_Get_address(&sample_person.height, &displacements[2]);
    MPI_Get_address(&sample_person.name, &displacements[3]);
    
    // Make relative to base_address
    for (int i = 0; i < 4; i++) {
        displacements[i] = MPI_Aint_diff(displacements[i], base_address);
    }
    
    // Create and commit the datatype
    MPI_Type_create_struct(4, blocks, displacements, types, &person_type);
    MPI_Type_commit(&person_type);

    Person person;
    
    if (world_rank == 0) {
        // Process 0 initializes person data
        person.id = 101;
        person.age = 30;
        person.height = 175.5;
        std::string name = "Alice";
        std::copy(name.begin(), name.end(), person.name);
        person.name[name.length()] = '\0';
        
        std::cout << std::format("Process 0 sending Person: id={}, age={}, height={}, name={}\n",
                               person.id, person.age, person.height, person.name);
                               
        // Send to process 1
        MPI_Send(&person, 1, person_type, 1, 0, MPI_COMM_WORLD);
    } else if (world_rank == 1) {
        // Process 1 receives the person data
        MPI_Status status;
        MPI_Recv(&person, 1, person_type, 0, 0, MPI_COMM_WORLD, &status);
        
        std::cout << std::format("Process 1 received Person: id={}, age={}, height={}, name={}\n",
                               person.id, person.age, person.height, person.name);
    }
    
    // Free the datatype
    MPI_Type_free(&person_type);
    
    MPI_Finalize();
    return 0;
}
```

### Parallel Computing Example: Matrix Multiplication

```cpp
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
```
