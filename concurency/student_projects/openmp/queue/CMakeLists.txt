cmake_minimum_required(VERSION 3.28)

project(queue CXX)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Corrected include directories
set(INCLUDE_DIRS
  ${CURRENT_CMAKE_SOURCE_DIR}/include
  ${CURRENT_CMAKE_SOURCE_DIR}/src/Benchmark
)

find_package(OpenMP REQUIRED)

# Create an executable
add_executable(${PROJECT_NAME} 
  ./src/main.cpp 
  ./src/Benchmark/omp_queue_test.cpp
  ./src/Benchmark/test_own_example.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})

target_compile_options(${PROJECT_NAME} PRIVATE
  -ftemplate-backtrace-limit=0
  -O3
  -Wall
  -Wextra
  -pedantic
)

target_link_libraries(${PROJECT_NAME}
  OpenMP::OpenMP_CXX
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
