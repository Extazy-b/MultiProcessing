#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <math.h>

const double a = 3.5;
const double b = 4.5;
const double max_f4 = 4.27394;

double f(double x) {
    return log(log(x) * sin(2 * x));
}

double simpson(double a, double b, int n) {
    double h = (b - a) / n;
    double sum = f(a) + f(b);
    for (int i = 1; i < n; i++) {
        double x = a + i * h;
        if (i % 2 == 0)
            sum += 2.0 * f(x);
        else
            sum += 4.0 * f(x);
    }
    return sum * h / 3.0;
}

int getN(double err, double a, double b, double max_f4) {
    double h_target = pow(err * 180.0 / (max_f4 * (b - a)), 0.25);
    double N_float = (b - a) / h_target;
    int N = (int)ceil(N_float);
    if (N % 2 != 0) N++;
    return N;
}

double parallel_simpson(double a, double b, int N, int K) {
    double start = omp_get_wtime();
    double h = (b - a) / N;
    double total = 0.0;

    #pragma omp parallel num_threads(K) reduction(+:total)
    {
        int tid = omp_get_thread_num();
        int nthreads = omp_get_num_threads();

        int base = N / nthreads;
        if (base % 2 != 0) base--;              
        int remainder = N - base * nthreads;     
        int extra = remainder / 2;                

        int local_n;         
        int start_idx, end_idx;

        if (tid < extra) {
            local_n = base + 2;
            start_idx = tid * (base + 2);
            end_idx = start_idx + local_n;
        } else {
            local_n = base;
            start_idx = tid * base + extra * 2;
            end_idx = start_idx + local_n;
        }

        double local_a = a + start_idx * h;
        double local_b = a + end_idx * h;

        double local_sum = 0.0;
        if (local_n > 0) {
            local_sum = simpson(local_a, local_b, local_n);
        }
        total += local_sum;
    }

    double end = omp_get_wtime();
    return end - start;
}

int main(int argc, char *argv[]) {
    double err = 0.000001;

    int N_seq = getN(err, a, b, max_f4);
    double integral_seq = simpson(a, b, N_seq);
    printf("%.10f, N = %d\n\n", integral_seq, N_seq);

    int p = omp_get_max_threads();
    int scales[] = {1, 10, 100, 1000, 10000, 100000};
    int num_scales = sizeof(scales) / sizeof(scales[0]);

    for (int s = 0; s < num_scales; s++) {
        printf("\tN=%d\t", scales[s] * N_seq);
    }
    printf("\n");

    for (int K = 1; K <= 2*p; K++) {
        printf("%d", K);
        for (int s = 0; s < num_scales; s++) {
            int N_curr = scales[s] * N_seq;
            if (N_curr % 2 != 0) N_curr++;
            double time = parallel_simpson(a, b, N_curr, K);
            printf("\t%.6f", time);
        }
        printf("\n");
    }

    return 0;
}