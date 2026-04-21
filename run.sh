#!/bin/bash

# Создаём папку для результатов, если её нет
mkdir -p results

# Функция для запуска OpenMP-программы
run_openmp() {
    local exe_path="$1"
    local out_path="$2"
    echo "Running $exe_path -> $out_path"
    "$exe_path" > "$out_path" 2>&1
}

# Функция для запуска MPI-программы (цикл по числу процессов)
run_mpi() {
    local exe_path="$1"
    local out_path="$2"
    echo "Running $exe_path with K=1..16 -> $out_path"
    # Очищаем файл перед записью
    > "$out_path"
    for K in {1..16}; do
        mpirun -np $K --oversubscribe "$exe_path" >> "$out_path" 2>&1
    done
    echo "" >> "$out_path"
}

# Запуск всех OpenMP-программ
echo "========================================="
echo "Running OpenMP programs (.1.exe)"
for exe in exe/*.1.exe; do
    if [ -f "$exe" ]; then
        name=$(basename "$exe" .1.exe)
        run_openmp "$exe" "results/${name}.1.txt"
    fi
done

# Запуск всех MPI-программ
echo "========================================="
echo "Running MPI programs (.2.exe)"
for exe in exe/*.2.exe; do
    if [ -f "$exe" ]; then
        name=$(basename "$exe" .2.exe)
        run_mpi "$exe" "results/${name}.2.txt"
    fi
done

echo "All runs completed."