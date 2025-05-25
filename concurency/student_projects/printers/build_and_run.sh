#!/bin/bash

# Exit immediately if a command fails
set -e

# Create and enter the build directory
BUILD_DIR="build"

if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# Run cmake to configure the build system
cmake ..

# Build the project
cmake --build .

# Run the program if build succeeds
echo "=== Build successful. Running the program ==="
./PrintersSimulation