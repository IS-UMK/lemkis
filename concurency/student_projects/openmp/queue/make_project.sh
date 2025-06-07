#!/bin/bash

# Dirs
BUILD_DIR="build"
SRC_DIR="src"

echo "pwd= " $pwd
# 1. Building the project
echo "Building the project..."
echo "Deleting the old build folder..."
rm -rf $BUILD_DIR

echo "Running cmake..."
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B $BUILD_DIR
cd $BUILD_DIR
# 2. Making the project
echo "Running make..."
make

# 3. Checking the project
cd ..
echo "Running clang-tidy..."
find ./src -iname '*.cxx' -o -iname '*.ixx' -o -iname '*.cpp'  -o -iname '*.hpp' | xargs clang-tidy-19  -p ./build/ --extra-arg=-std=c++26 --config-file=./.clang-tidy

