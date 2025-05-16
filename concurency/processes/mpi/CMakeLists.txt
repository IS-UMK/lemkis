set(MPI_CXX_COMPILER "/usr/lib64/mpi/gcc/openmpi5/bin/mpic++" CACHE FILEPATH "MPI C++ compiler wrapper")

include_directories(${CMAKE_SOURCE_DIR}/external/mpi/include)
find_package(MPI REQUIRED)
add_executable(mpi mpi.cpp)
target_link_libraries(mpi PUBLIC MPI::MPI_CXX) 