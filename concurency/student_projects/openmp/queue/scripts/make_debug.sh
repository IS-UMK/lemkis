#!/bin/bash

# Directory variables for build and source code
BUILD_DIR="../build"

# Step 1: Build the project
echo "Building the project..."

# Remove any existing build directory to start fresh
echo "Deleting the old build folder..."
rm -rf $BUILD_DIR

# Configure the project with CMake, enabling compilation database export
echo "Running cmake..."
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B $BUILD_DIR



