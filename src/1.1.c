#include <stdio.h>
#include <unistd.h>
#include <omp.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int K = 16;
    omp_set_num_threads(K);

    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int total_threads = omp_get_num_threads();

        printf("Hello! I am thread %d from %d threads.\n", thread_id, total_threads);

        sleep(5);

        if (thread_id % 2 == 0) {
            printf("Hello again! I am thread %d. I am even.\n", thread_id);
        }
    }
    printf("Finish.\n");
}