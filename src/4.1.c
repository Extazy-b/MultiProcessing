// g++ -fopenmp src/4.1.c -o exe/4.1.exe; exe/4.1.c > results/4.1.txt

#include <stdio.h>
#include <omp.h>
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

double parallel_pi(int count, int K) {
    double start = omp_get_wtime();
    double total = 0.0;

    #pragma omp parallel num_threads(K) reduction(+:total)
    {
        int tid = omp_get_thread_num();
        int nthreads = omp_get_num_threads();

        int base = count / nthreads;
        int rem = count % nthreads;

        int start_idx, end_idx;
        if (tid < rem) {
            start_idx = tid * (base + 1);
            end_idx = start_idx + base;
        } else {
            start_idx = rem * (base + 1) + (tid - rem) * base;
            end_idx = start_idx + base - 1;
        }

        total += range_sum(start_idx, end_idx);
    }

    double end = omp_get_wtime();
    return end - start;
}

int main() {
    double eps = 1e-4;
    unsigned int count_seq = getCount(eps);
    double pi_seq = 4.0 * range_sum(0, count_seq - 1);

    printf("Sequential: pi = %.10f, N = %d\n\n", pi_seq, count_seq);

    int p = omp_get_max_threads();
    int scales[] = {1, 10, 100, 1000, 10000, 100000};
    int num_scales = sizeof(scales) / sizeof(scales[0]);

    for (int s = 0; s < num_scales; ++s)
        printf("\tN=%d\t", scales[s] * count_seq);
    printf("\n");

    for (int K = 1; K <= 2 * p; ++K) {
        printf("%d", K);
        for (int s = 0; s < num_scales; ++s) {
            int count_curr = scales[s] * count_seq;
            double time = parallel_pi(count_curr, K);
            printf("\t%.6f", time);
        }
        printf("\n");
    }

    return 0;
}