#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

double parallel_matmul(int n, int K) {
    double *A = (double*)malloc(n * n * sizeof(double));
    double *B = (double*)malloc(n * n * sizeof(double));
    double *C = (double*)calloc(n * n, sizeof(double));

    for (int i = 0; i < n * n; i++) {
        A[i] = (double)rand() / RAND_MAX;
        B[i] = (double)rand() / RAND_MAX;
    }

    double start = omp_get_wtime();

    #pragma omp parallel num_threads(K)
    {
        int tid = omp_get_thread_num();
        int nthreads = omp_get_num_threads();

        int base_rows = n / nthreads;
        int rem = n % nthreads;
        int local_rows = (tid < rem) ? base_rows + 1 : base_rows;
        int start_row = (tid < rem) ? tid * (base_rows + 1) : rem * (base_rows + 1) + (tid - rem) * base_rows;
        int end_row = start_row + local_rows;

        for (int i = start_row; i < end_row; i++) {
            for (int k = 0; k < n; k++) {
                double aik = A[i * n + k];
                for (int j = 0; j < n; j++) {
                    C[i * n + j] += aik * B[k * n + j];
                }
            }
        }
    }

    double end = omp_get_wtime();
    double elapsed = end - start;

    free(A);
    free(B);
    free(C);
    return elapsed;
}

int main() {
    int p = omp_get_max_threads();
    int maxK = 2 * p;
    int sizes[] = {100, 1000, 3000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    printf("Matrix multiplication time (seconds)\n");
    for (int s = 0; s < num_sizes; s++)
        printf("\t\t%d\t", sizes[s]);
    printf("\n");

    for (int K = 1; K <= maxK; K++) {
        printf("%d", K);
        for (int s = 0; s < num_sizes; s++) {
            int n = sizes[s];
            double time = parallel_matmul(n, K);
            printf("\t%.6f", time);
        }
        printf("\n");
    }
    return 0;
}