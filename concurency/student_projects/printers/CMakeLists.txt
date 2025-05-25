cmake_minimum_required(VERSION 3.23)
project(concurrency C CXX)

set(CMAKE_CXX_STANDARD 26)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++26")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra --pedantic -g3 -ggdb -fexperimental-library")

add_executable(PrintersSimulation main.cpp)

# Make headers in lib/ accessible
target_include_directories(PrintersSimulation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib)