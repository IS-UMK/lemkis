cmake_minimum_required(VERSION 3.26)
project(cafe_modules LANGUAGES CXX)

# Kompilator: tylko Clang
if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  message(FATAL_ERROR "Musisz używać Clang, np. clang++-17")
endif()

# Standard C++
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Ścieżki do nagłówków
set(INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(SOURCES
  src/main.cpp
  src/cafe/Cafe.cpp
  src/cafe/Process.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})

# Flagi kompilatora
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23 -ftemplate-backtrace-limit=0 -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall --pedantic -g3 -ggdb")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23 -ftemplate-backtrace-limit=0 -O3 -stdlib=libc++")

# Instalacja
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

#cmake -G Ninja -B build -S . -DCMAKE_CXX_COMPILER=clang++
