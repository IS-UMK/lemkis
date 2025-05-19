cmake_minimum_required(VERSION 3.28)

project(queue CXX)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For newer clang versions
if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    execute_process(COMMAND ${CMAKE_C_COMPILER} --version OUTPUT_VARIABLE CLANG_VERSION)
    string(REGEX MATCH "version [0-9]+" CLANG_VERSION ${CLANG_VERSION})
    string(REGEX MATCH "[0-9]+" CLANG_VERSION ${CLANG_VERSION})
    
    set(OpenMP_C_FLAGS "-fopenmp=libomp")
    set(OpenMP_C_LIB_NAMES "omp")
    set(OpenMP_omp_LIBRARY "/usr/lib/libomp.so")
    if(EXISTS "/usr/lib/llvm-${CLANG_VERSION}/lib/libomp.so")
        set(OpenMP_omp_LIBRARY "/usr/lib/llvm-${CLANG_VERSION}/lib/libomp.so")
    endif()
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(OpenMP_CXX_FLAGS "-fopenmp=libomp")
    set(OpenMP_CXX_LIB_NAMES "omp")
endif()
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
