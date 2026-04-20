cmake_minimum_required(VERSION 3.23)
project(group_synchronization C CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++26")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra --pedantic -g3 -ggdb -fexperimental-library")


add_executable(
  ${PROJECT_NAME}
  ${PROJECT_SOURCE_DIR}/main.cpp
  )

target_link_libraries(
  ${PROJECT_NAME}
  stdc++
  )
