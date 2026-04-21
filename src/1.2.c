#include <stdio.h>
#include <unistd.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // printf("Hello! I'm thread %d from %d threads.\n", thread_num, size);
    printf("Hello! I'm thread %d from %d threads (PID=%d).\n", rank, size, getpid());
    fflush(stdout);

    sleep(5);

    if (rank % 2 == 0) {
        printf("Hello again! I'm thread %d. I'm even.\n", rank);
        fflush(stdout);
    }

    MPI_Finalize();
    return 0;
}