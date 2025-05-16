cmake_minimum_required(VERSION 3.28)

project(queue CXX)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Lista plików źródłowych
set(SOURCES
  ${CMAKE_SOURCE_DIR}/src/main.cpp
  ${CMAKE_SOURCE_DIR}/src/Benchmark/omp_queue_test.cpp
  ${CMAKE_SOURCE_DIR}/src/Benchmark/test_own_example.cpp
)

# Katalogi nag��wk�w
set(INCLUDE_DIRS
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src/Benchmark
)

# Znajd� bibliotek� OpenMP
find_package(OpenMP REQUIRED)

# Utw�rz plik wykonywalny
add_executable(${PROJECT_NAME} ${SOURCES})

# Do��cz katalogi nag��wk�w
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})

# Opcje kompilatora
target_compile_options(${PROJECT_NAME} PRIVATE
  -ftemplate-backtrace-limit=0
  # -ffast-math
  -Ofast
)

# Linkowanie bibliotek
target_link_libraries(${PROJECT_NAME}
  OpenMP::OpenMP_CXX
  tbb
)

# Instalacja binarki
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
