cmake_minimum_required(VERSION 3.28)

project(omp_benchmark_queue CXX)


include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type to Release
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)

# List of source files
set(SOURCES
  ${CMAKE_SOURCE_DIR}/src/main.cpp
  ${CMAKE_SOURCE_DIR}/src/Benchmark/omp_queue_test.cpp
  ${CMAKE_SOURCE_DIR}/src/Benchmark/test_own_example.cpp
)

# Corrected include directories
set(INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src/Benchmark
)

find_package(OpenMP REQUIRED)

# Create an executable
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})

target_compile_options(${PROJECT_NAME} PRIVATE
  -O3 
  -march=native
  -ftree-parallelize-loops=4
)

target_link_libraries(${PROJECT_NAME}
  OpenMP::OpenMP_CXX
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
