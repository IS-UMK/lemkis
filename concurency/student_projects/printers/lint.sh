#!/bin/bash

set -e

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}Running clang-format...${NC}"

# Files to process: all headers in lib and main.cpp
clang_format_targets=$(find lib -name "*.h" -o -name "*.hpp")
clang_format_targets+=" main.cpp"

# Apply clang-format using .clang-format from current directory
for file in $clang_format_targets; do
    clang-format -i --style=file "$file"
    echo "Formatted: $file"
done

echo -e "${GREEN}Generating compile_commands.json...${NC}"

rm -rf build

BUILD_DIR="build"
if [ ! -d "$BUILD_DIR" ]; then
    cmake -S . -B "$BUILD_DIR" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
fi

echo -e "${GREEN}Running clang-tidy...${NC}"

for file in $clang_format_targets; do
    echo "Analyzing: $file"
    clang-tidy "$file" \
      --quiet \
      --config-file=.clang-tidy \
      --warnings-as-errors='*' \
      --extra-arg=-std=c++20 \
      --extra-arg=-I/usr/include/c++/$(g++ -dumpversion) \
      --extra-arg=-I/usr/include/x86_64-linux-gnu/c++/$(g++ -dumpversion) \
      -p "$BUILD_DIR"
done

echo -e "${GREEN}All linting complete.${NC}"
