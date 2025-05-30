cmake_minimum_required(VERSION 3.23)
project(concurrency C CXX)

set(CMAKE_CXX_STANDARD 26)
add_subdirectory(lib)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++26")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra --pedantic -g3 -ggdb")

add_executable(PrintersSimulation main.cpp)

target_link_libraries(PrintersSimulation PRIVATE PrintersLib)