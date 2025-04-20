cmake_minimum_required(VERSION 3.28)

project(queue CXX)

include(CTest)
enable_testing()

add_executable(${PROJECT_NAME} main.cxx)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
  $(CMAKE_SOURCE_DIR)/src/main.cpp
)

set(INCLUDE_DIRS
  $(CMAKE_SOURCE_DIR)/src/include
)

find_package(OpenMP REQUIRED)

# Create an executable
add_executable(${PROJECT_NAME}
	${SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23 -ftemplate-backtrace-limit=0 -O3 ")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_X_FLAGS_DEBUG} -Wall --pedantic -g3 -ggdb")

target_link_libraries(${PROJECT_NAME}
    OpenMP::OpenMP_CXX
)


install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
