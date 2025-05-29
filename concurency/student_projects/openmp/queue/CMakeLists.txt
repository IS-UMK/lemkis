cmake_minimum_required(VERSION 3.28)

project(queue CXX)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Find libomp dynamically
execute_process(
  COMMAND find /usr -name "libomp.so*" 
  OUTPUT_VARIABLE LIBOMP_PATHS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)


# Parse the output to get the first path
string(REPLACE "\n" ";" LIBOMP_PATH_LIST "${LIBOMP_PATHS}")
list(GET LIBOMP_PATH_LIST 0 LIBOMP_PATH)
get_filename_component(LIBOMP_DIR "${LIBOMP_PATH}" DIRECTORY)

# Set OpenMP configuration
set(OpenMP_C_FLAGS "-fopenmp" CACHE STRING "OpenMP C flags")
set(OpenMP_CXX_FLAGS "-fopenmp" CACHE STRING "OpenMP C++ flags") 
set(OpenMP_C_LIB_NAMES "omp" CACHE STRING "OpenMP C libraries")
set(OpenMP_CXX_LIB_NAMES "omp" CACHE STRING "OpenMP C++ libraries")
set(OpenMP_omp_LIBRARY "${LIBOMP_PATH}" CACHE FILEPATH "Path to the omp library")

# Add the directory to runtime path
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,${LIBOMP_DIR}")

message(STATUS "Found libomp at: ${LIBOMP_PATH}")
message(STATUS "Adding rpath: ${LIBOMP_DIR}")
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
  ${CURRENT_CMAKE_SOURCE_DIR}/src/include
  ${CURRENT_CMAKE_SOURCE_DIR}/src/Benchmark
)

find_package(OpenMP REQUIRED)
find_package(TBB REQUIRED)


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
 TBB::tbb
 )

target_link_libraries(${PROJECT_NAME}
  OpenMP::OpenMP_CXX
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
