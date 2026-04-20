#!/bin/bash

# Directory variables for build and source code
BUILD_DIR="../build"
SRC_DIR="../src"

# Change to the build directory to start compilation
cd $BUILD_DIR

# Step 2: Compile the project using make
echo "Running make..."
make
