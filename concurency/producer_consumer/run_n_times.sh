#!/bin/bash
set -e  # Wyjście przy błędzie

EXE_NAME="${1:-mutex}"
RUNS="${2:-1}"

if ! [[ "$RUNS" =~ ^[1-9][0-9]*$ ]]; then
    echo "Użycie: $0 [nazwa_programu] [liczba_uruchomień>0]" >&2
    exit 1
fi

SRC_FILE="${EXE_NAME}.cpp"

echo "=== KOMPILUJĘ: $SRC_FILE -> $EXE_NAME ==="
g++ -std=c++23 -Wall -Wextra -pthread -O0 "$SRC_FILE" -o "$EXE_NAME"

for ((i = 1; i <= RUNS; ++i)); do
    echo "=== URUCHAMIAM ($i/$RUNS) ==="
    timeout 10 ./"$EXE_NAME"
    echo "=== EXIT CODE: $? ==="
done

