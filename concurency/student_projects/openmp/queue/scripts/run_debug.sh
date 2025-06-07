#!/bin/bash

# Directory variables for build and source code
BUILD_DIR="../build"
SRC_DIR="../src"

# Change to the build directory to start compilation
cd $BUILD_DIR

# Step 2: Compile the project using make
echo "Running make..."
make

# Step 3: Run clang-tidy static analysis on source files
#cd ..

#echo "Running clang-tidy..."
# Find all relevant C++ source and header files, then run clang-tidy with the specified config and compilation database
#find $SRC_DIR -iname '*.cxx' -o -iname '*.ixx' -o -iname '*.cpp' -o -iname '*.hpp' | xargs clang-tidy-19 -p $BUILD_DIR --extra-arg=-std=c++26 --config-file=./.clang-tidy

