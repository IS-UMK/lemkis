cmake_minimum_required(VERSION 3.28)
# set(MPI_CXX_COMPILER "/usr/bin/mpic++" CACHE FILEPATH "MPI C++ compiler wrapper")
# set(CXX_COMPILER "/usr/local/bin/g++" CACHE FILEPATH "MPI C++ compiler wrapper")
# set(C_COMPILER "/usr/local/bin/gcc" CACHE FILEPATH "MPI C++ compiler wrapper")

# set (CXX)
# Set C++23 as the standard for all targets
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # Use -std=c++23 instead of -std=gnu++23
# include_directories(${CMAKE_SOURCE_DIR}/external/mpi/include)
find_package(MPI REQUIRED COMPONENTS CXX)

# add_executable(helper helper.cpp)
# target_link_libraries(helper PRIVATE MPI::MPI_CXX)


# add_executable(mpi mpi.cpp)
# target_link_libraries(mpi PUBLIC MPI::MPI_CXX) 

add_executable(broadcast broadcast.cpp)
target_link_libraries(broadcast PUBLIC MPI::MPI_CXX)

add_executable(hello_world hello_world.cpp)
target_link_libraries(hello_world PUBLIC MPI::MPI_CXX)

add_executable(matrix_multiplication matrix_multiplication.cpp)
target_link_libraries(matrix_multiplication PUBLIC MPI::MPI_CXX)

add_executable(point_to_point point_to_point.cpp)
target_link_libraries(point_to_point PUBLIC MPI::MPI_CXX)

add_executable(reduce reduce.cpp)
target_link_libraries(reduce PUBLIC MPI::MPI_CXX)

add_executable(custom_data custom_data.cpp)
target_link_libraries(custom_data PUBLIC MPI::MPI_CXX)

add_executable(helper helper.cpp)
target_link_libraries(helper PUBLIC MPI::MPI_CXX)

add_executable(mpi mpi.cpp)
target_link_libraries(mpi PUBLIC MPI::MPI_CXX)

add_executable(point_to_point_non_blocking point_to_point_non_blocking.cpp)
target_link_libraries(point_to_point_non_blocking PUBLIC MPI::MPI_CXX)

add_executable(scatter_and_gather scatter_and_gather.cpp)
target_link_libraries(scatter_and_gather PUBLIC MPI::MPI_CXX)
