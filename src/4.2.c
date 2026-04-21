// mpicc -o exe/4.2.exe src/4.2.c -lm; echo > results/4.2.txt; for K in {1..16}; do mpirun -np $K --oversubscribe exe/4.2.exe >> results/4.2.txt; done;  echo >> results/4.2.txt

#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>

double term(int n) {
    return (n % 2 == 0 ? 1.0 : -1.0) / (2.0 * n + 1.0);
}

double range_sum(int start, int end) {
    double sum = 0.0;
    for (int n = start; n <= end; ++n)
        sum += term(n);
    return sum;
}

unsigned int getCount(double eps) {
    // |R_n| < 1/(2n+1) < eps  =>  n > (1/eps - 1)/2
    unsigned int n = (unsigned int)(0.5 * (1.0 / eps - 1.0)) + 1;
    return n;
}

double parallel_pi(int count, MPI_Comm comm, int rank, double *elapsed_time) {
    double start = MPI_Wtime();
    
    int size;
    MPI_Comm_size(comm, &size);

    int base = count / size;
    int rem = count % size;
    
    int start_idx, end_idx;
    if (rank < rem) {
        start_idx = rank * (base + 1);
        end_idx = start_idx + base;
    } else {
        start_idx = rem * (base + 1) + (rank - rem) * base;
        end_idx = start_idx + base - 1;
    }
    
    double local_sum = 0.0;
    if (start_idx <= end_idx) {
        local_sum = range_sum(start_idx, end_idx);
    }
    
    double total_sum = 0.0;
    MPI_Reduce(&local_sum, &total_sum, 1, MPI_DOUBLE, MPI_SUM, 0, comm);
    
    double end = MPI_Wtime();
    if (rank == 0) *elapsed_time = end - start;
    
    return 4.0 * total_sum;
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    double eps = 1e-4;
    
    unsigned int count_seq = 0;
    double pi_seq = 0.0;
    if (rank == 0) {
        count_seq = getCount(eps);
        pi_seq = 4.0 * range_sum(0, count_seq - 1);
        if (size == 1) printf("Sequential: pi = %.10f, N = %d\n\n", pi_seq, count_seq);
    }
    
    MPI_Bcast(&count_seq, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    int scales[] = {1, 10, 100, 1000, 10000, 100000};
    int num_scales = sizeof(scales) / sizeof(scales[0]);
    
    if (rank == 0) {
        if (size == 1) for (int s = 0; s < num_scales; ++s) {
            if (size == 1) printf("\tN=%d\t", scales[s] * count_seq);
        }
        if (size == 1) printf("\n");
        printf("%d", size);
    }
    
    for (int s = 0; s < num_scales; ++s) {
        int count_curr = scales[s] * count_seq;
        double elapsed = 0.0;
        double pi_par = parallel_pi(count_curr, MPI_COMM_WORLD, rank, &elapsed);
        if (rank == 0) {
            printf("\t%.6f", elapsed);
            fflush(stdout);
        }
    }
    if (rank == 0) printf("\n");
    
    MPI_Finalize();
    return 0;
}