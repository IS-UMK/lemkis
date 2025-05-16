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

add_executable(helper helper.cpp)
target_link_libraries(helper PRIVATE MPI::MPI_CXX)


add_executable(mpi mpi.cpp)
target_link_libraries(mpi PUBLIC MPI::MPI_CXX) 