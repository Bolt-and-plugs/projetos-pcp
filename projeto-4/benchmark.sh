#!/bin/bash

# Compile
make

INPUT="assets/realdata.dat"
OUTPUT_SEQ="assets/output/output_seq.dat"
OUTPUT_MPI="assets/output/output_mpi.dat"
TIME_FILE="assets/output/time_measure.txt"

# Ensure output directory exists
mkdir -p assets/output

# 1. Sequential (1 process)
echo "Running Sequential (1 process)..."
for i in {1..3}; do
    echo "  Run $i..."
    ./seq --input_path $INPUT --output_path $OUTPUT_SEQ
done

# 2. MPI (1 process)
echo "Running MPI (1 process)..."
for i in {1..3}; do
    echo "  Run $i..."
    mpirun --oversubscribe -np 1 ./mpi --input_path $INPUT --output_path $OUTPUT_MPI --num_cli 8 --block_x 128 --block_y 128
done

# 3. MPI (2 processes)
echo "Running MPI (2 processes)..."
for i in {1..3}; do
    echo "  Run $i..."
    mpirun --oversubscribe -np 2 ./mpi --input_path $INPUT --output_path $OUTPUT_MPI --num_cli 8 --block_x 128 --block_y 128
done

# 4. MPI (4 processes)
echo "Running MPI (4 processes)..."
for i in {1..3}; do
    echo "  Run $i..."
    mpirun --oversubscribe -np 4 ./mpi --input_path $INPUT --output_path $OUTPUT_MPI --num_cli 8 --block_x 128 --block_y 128
done

# 5. MPI (8 processes)
echo "Running MPI (8 processes)..."
for i in {1..3}; do
    echo "  Run $i..."
    mpirun --oversubscribe -np 8 ./mpi --input_path $INPUT --output_path $OUTPUT_MPI --num_cli 8 --block_x 128 --block_y 128
done

echo "Benchmark completed. Results in $TIME_FILE"
