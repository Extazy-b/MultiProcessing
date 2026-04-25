#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int isPrime(int n) {
    if (n < 2) return 0;
    for (int i = 2; i <= sqrt(n); i++)
        if (n % i == 0) return 0;
    return 1;
}
int countPrimesInRange(int start, int end) {
    int count = 0;
    for (int n = start; n < end; n++)
        if (isPrime(n)) count++;
    return count;
}
double parallel_count_primes_mpi(int M, int N, int rank, int size) {
    int total = N - M + 1; 

    int base = total / size;
    int rem = total % size;
    int local_total = (rank < rem) ? base + 1 : base;
    int start_idx = (rank < rem) ? rank * (base + 1) : rem * (base + 1) + (rank - rem) * base;
    int end_idx = start_idx + local_total;

    int local_M = M + start_idx;
    int local_N = M + end_idx;

    double start_time = MPI_Wtime();
    int local_count = countPrimesInRange(local_M, local_N);
    double end_time = MPI_Wtime();
    double elapsed = end_time - start_time;


    return elapsed;
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int M = 10;
    const int N = 300;
    int scales[] = {1, 10, 100, 1000, 10000};
    int num_scales = sizeof(scales) / sizeof(scales[0]);

    if (rank == 0) {
        int seq_count = countPrimesInRange(M, N + 1);
        if (size == 1) printf("Sequential: total primes in [%d, %d] = %d\n\n", M, N, seq_count);
    }

    if (rank == 0) {
        if (size == 1) printf("Prime counting time (seconds)\n");
        if (size == 1) for (int s = 0; s < num_scales; s++) {
            int low = M * scales[s];
            int high = N * scales[s];
            printf("\t[%d, %d]\t", low, high);
        }
        if (size == 1) printf("\n");
        printf("%d", size); 
    }

    for (int s = 0; s < num_scales; s++) {
        int low = M * scales[s];
        int high = N * scales[s];
        double time = parallel_count_primes_mpi(low, high, rank, size);
        if (rank == 0) {
            printf("\t%.6f", time);
            fflush(stdout);
        }
    }
    if (rank == 0) printf("\n");

    MPI_Finalize();
    return 0;
}