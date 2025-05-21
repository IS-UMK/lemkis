set(CMAKE_CXX_COMPILER "g++-14")

cmake_minimum_required(VERSION 3.26)
project(StackAndQueue C CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# find_program(CLANG_TIDY NAMES clang-tidy)
# if(CLANG_TIDY)
#   set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY}")
#   message(STATUS "Clang-tidy enabled")
# else()
#   message(WARNING "Clang-tidy not found, checks disabled")
# endif()

set(SOURCES
  ${CMAKE_SOURCE_DIR}/src/main.cpp
  ${CMAKE_SOURCE_DIR}/src/struct_test.cpp
  ${CMAKE_SOURCE_DIR}/src/old_benchmark.cpp
)

set(INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/src/include
  ${CMAKE_SOURCE_DIR}/src/libs
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++26 -ftemplate-backtrace-limit=0 -O3 ")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_X_FLAGS_DEBUG} -Wall --pedantic -g3 -ggdb")

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
target_compile_options(StackAndQueue PRIVATE -std=c++2c)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)