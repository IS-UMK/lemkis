cmake_minimum_required(VERSION 3.23)
project(concurrency C CXX)

set(CMAKE_CXX_STANDARD 26)
add_subdirectory(./mutex)
add_subdirectory(./compare_and_swap)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++26")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra --pedantic -g3 -ggdb")


add_executable(
  ${PROJECT_NAME}
  ${PROJECT_SOURCE_DIR}/main.cpp
  )

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} pthread mutex cas)
