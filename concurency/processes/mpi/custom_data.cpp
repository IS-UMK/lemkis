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