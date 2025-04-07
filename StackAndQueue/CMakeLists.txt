cmake_minimum_required(VERSION 3.23)
project(StackAndQueue C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23 -ftemplate-backtrace-limit=0")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_X_FLAGS_DEBUG} -Wall --pedantic -g3 -ggdb")

include_directories(${CMAKE_SOURCE_DIR}/src/include)

add_executable(${PROJECT_NAME}
  ${PROJECT_SOURCE_DIR}/main.cpp
  ${PROJECT_SOURCE_DIR}/src/stack_test.cpp
  ${PROJECT_SOURCE_DIR}/src/queue_test.cpp
  ${PROJECT_SOURCE_DIR}/src/benchmark.cpp
  )

set(CMAKE_CXX_CLANG_TIDY
  clang-tidy-11;
  -format-style='file';
  -header-filter=${CMAKE_CURRENT_SOURCE_DIR};
)
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}" "-checks=*")
endif()