#!/bin/bash
set -e  # Wyjście przy błędzie

EXE_NAME="${1:-mutex}"
SRC_FILE="${EXE_NAME}.cpp"

echo "=== KOMPILUJĘ: $SRC_FILE -> $EXE_NAME ==="
g++ -std=c++23 -Wall -Wextra -pthread -O0 "$SRC_FILE" -o "$EXE_NAME"

echo "=== URUCHAMIAM ==="
# Unbuffered output + timeout 10s
timeout 10 ./$EXE_NAME 
echo "=== EXIT CODE: $? ==="
