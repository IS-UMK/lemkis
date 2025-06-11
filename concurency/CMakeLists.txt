cmake_minimum_required(VERSION 3.23)
project(concurrency C CXX)

set(CMAKE_CXX_STANDARD 26)
add_subdirectory(./mutex)
add_subdirectory(./compare_and_swap)
# add_subdirectory(./student_projects/openmp/queue)
add_subdirectory(./student_projects/StackAndQueue)
# add_subdirectory(./processes/mpi)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++26")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra --pedantic -g3 -ggdb -fexperimental-library")


add_executable(
  ${PROJECT_NAME}
  ${PROJECT_SOURCE_DIR}/main.cpp
  )

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} pthread mutex cas)
