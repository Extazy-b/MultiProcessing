// mpicc -o exe/3.2.exe src/3.2.c -lm; echo > results/3.2.txt; for K in {1..16}; do mpirun -np $K --oversubscribe exe/3.2.exe >> results/3.2.txt; done;  echo >> results/3.2.txt

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

double parallel_matmul(int n, int rank, int size) {
    double *A = (double*)malloc(n * n * sizeof(double));
    double *B = (double*)malloc(n * n * sizeof(double));
    double *C = (double*)calloc(n * n, sizeof(double));

    if (rank == 0) {
        for (int i = 0; i < n * n; i++) {
            A[i] = (double)rand() / RAND_MAX;
            B[i] = (double)rand() / RAND_MAX;
        }
    }

    MPI_Bcast(A, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(B, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    double start_time = MPI_Wtime();

    int base_rows = n / size;
    int rem = n % size;
    int local_rows = (rank < rem) ? base_rows + 1 : base_rows;
    int start_row = (rank < rem) ? rank * (base_rows + 1) : rem * (base_rows + 1) + (rank - rem) * base_rows;
    int end_row = start_row + local_rows;

    for (int i = start_row; i < end_row; i++) {
        for (int k = 0; k < n; k++) {
            double aik = A[i * n + k];
            for (int j = 0; j < n; j++) {
                C[i * n + j] += aik * B[k * n + j];
            }
        }
    }

    double end_time = MPI_Wtime();
    double elapsed = end_time - start_time;

    free(A);
    free(B);
    free(C);
    return elapsed;
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int sizes[] = {100, 1000, 3000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    if (rank == 0) {
        if (size == 1) printf("Matrix multiplication time (seconds)\n");
        if (size == 1) for (int s = 0; s < num_sizes; s++)
            printf("\t\t%d\t", sizes[s]);
        if (size == 1) printf("\n");
        printf("%d", size);
    }

    for (int s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        double time = parallel_matmul(n, rank, size);
        if (rank == 0) {
            printf("\t%.6f", time);
            fflush(stdout);
        }
    }
    if (rank == 0) printf("\n");

    MPI_Finalize();
    return 0;
}