#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

bool isPrime(int n) {
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

double parallel_count_primes(int M, int N, int K) {
    int total = N - M + 1;
    double start = omp_get_wtime();
    int global_count = 0;

    #pragma omp parallel num_threads(K) reduction(+:global_count)
    {
        int tid = omp_get_thread_num();
        int nthreads = omp_get_num_threads();

        int base = total / nthreads;
        int rem = total % nthreads;
        int local_total = (tid < rem) ? base + 1 : base;
        int start_idx = (tid < rem) ? tid * (base + 1) : rem * (base + 1) + (tid - rem) * base;
        int end_idx = start_idx + local_total;

        int local_M = M + start_idx;
        int local_N = M + end_idx;
        global_count += countPrimesInRange(local_M, local_N);
    }

    double end = omp_get_wtime();
    return end - start;
}

int main() {
    const int M = 10;
    const int N = 300;
    int scales[] = {1, 10, 100, 1000, 10000};
    int num_scales = sizeof(scales) / sizeof(scales[0]);

    int seq_count = countPrimesInRange(M, N + 1);
    printf("Sequential: total primes in [%d, %d] = %d\n\n", M, N, seq_count);

    int p = omp_get_max_threads();
    int maxK = 2 * p;

    printf("Prime counting time (seconds)\n");
    for (int s = 0; s < num_scales; s++) {
        int low = M * scales[s];
        int high = N * scales[s];
        printf("\t[%d, %d]\t", low, high);
    }
    printf("\n");

    for (int K = 1; K <= maxK; K++) {
        printf("%d", K);
        for (int s = 0; s < num_scales; s++) {
            int low = M * scales[s];
            int high = N * scales[s];
            double time = parallel_count_primes(low, high, K);
            printf("\t%.6f", time);
        }
        printf("\n");
    }

    return 0;
}