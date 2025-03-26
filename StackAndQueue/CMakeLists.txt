cmake_minimum_required(VERSION 3.23)
project(StackAndQueue C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_COMPILER "clang++-19")

add_subdirectory(./src)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23  -ftemplate-backtrace-limit=0")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra --pedantic -g3 -ggdb")

add_executable(
  ${PROJECT_NAME}
  ${PROJECT_SOURCE_DIR}/src/main.cpp
  )