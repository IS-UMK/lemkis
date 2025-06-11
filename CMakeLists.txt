cmake_minimum_required(VERSION 3.28)

project(lemkis CXX)

include(CTest)
enable_testing()

# Turning off extensions avoids an issue with the clang 16 compiler
# clang 17 and greater can avoid this setting
# set(CMAKE_CXX_EXTENSIONS OFF)
# Set the version of C++ for the project
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)		
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -std=c++26 -fexperimental-library")
# Create a library
## add_subdirectory(./algebra2)add_subdirectory(./discrete_math)add_subdirectory(./tests)

#  
# 
# 
add_subdirectory(./concurency)


# Create an executable
add_executable(${PROJECT_NAME} main.cxx)
# Link to the library foo
target_link_libraries(${PROJECT_NAME} libmatrix libexpect libeigen)

