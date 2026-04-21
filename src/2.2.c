// echo > results/2.2.txt; for K in {1..16}; do mpirun -np $K --oversubscribe exe/2.2.exe >> results/2.2.txt; done;  echo >> results/2.2.txt

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

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

double parallel_simpson(double a, double b, int N, int size, int rank, double *elapsed_time) {
    double start_time = MPI_Wtime();


    int base = N / size;
    if (base % 2 != 0) base--;
    int remainder = N - base * size;
    int extra = remainder / 2;

    int local_n;
    int start_idx, end_idx;

    if (rank < extra) {
        local_n = base + 2;
        start_idx = rank * (base + 2);
        end_idx = start_idx + local_n;
    } else {
        local_n = base;
        start_idx = rank * base + extra * 2;
        end_idx = start_idx + local_n;
    }

    double h = (b - a) / N;
    double local_a = a + start_idx * h;
    double local_b = a + end_idx * h;

    double local_sum = 0.0;
    if (local_n > 0) {
        local_sum = simpson(local_a, local_b, local_n);
    }

    double total_sum = 0.0;
    MPI_Reduce(&local_sum, &total_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime();
    if (rank == 0) *elapsed_time = end_time - start_time;

    return total_sum;
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    double err = 0.000001;
    int N_seq = 0;
    double integral_seq = 0.0;

    if (rank == 0) {
        N_seq = getN(err, a, b, max_f4);
        integral_seq = simpson(a, b, N_seq);
        if (size == 1) printf("Integral = %.10f, N = %d\n\n", integral_seq, N_seq);
    }

    MPI_Bcast(&N_seq, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int scales[] = {1, 10, 100, 1000, 10000, 100000};
    int num_scales = sizeof(scales) / sizeof(scales[0]);

    if (rank == 0) {
        for (int s = 0; s < num_scales; s++) {
            if (size == 1) printf("\tN=%d\t", scales[s] * N_seq);
        }
        printf("\n");
        printf("%d", size);
    }

    for (int s = 0; s < num_scales; s++) {
        int N_curr = scales[s] * N_seq;
        if (N_curr % 2 != 0) N_curr++;
        double elapsed = 0.0;
        double res = parallel_simpson(a, b, N_curr, size, rank, &elapsed);
        if (rank == 0) {
            printf("\t%.6f", elapsed);
            fflush(stdout);
        }
    }

    MPI_Finalize();
    return 0;
}