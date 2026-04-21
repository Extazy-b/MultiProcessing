#!/bin/bash

# Создаём папку для исполняемых файлов, если её нет
mkdir -p exe

# Компиляция OpenMP-версий (.1.c)
echo "Compiling OpenMP programs..."
for src in src/*.1.c; do
    if [ -f "$src" ]; then
        name=$(basename "$src" .1.c)
        echo "  $src -> exe/${name}.1.exe"
        g++ -fopenmp "$src" -o "exe/${name}.1.exe" -lm
        if [ $? -ne 0 ]; then
            echo "Error compiling $src"
        fi
    fi
done

# Компиляция MPI-версий (.2.c)
echo "Compiling MPI programs..."
for src in src/*.2.c; do
    if [ -f "$src" ]; then
        name=$(basename "$src" .2.c)
        echo "  $src -> exe/${name}.2.exe"
        mpicc "$src" -o "exe/${name}.2.exe" -lm
        if [ $? -ne 0 ]; then
            echo "Error compiling $src"
        fi
    fi
done

echo "Compilation finished."