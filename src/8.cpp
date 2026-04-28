

#include <mpi.h>
// #include <chrono>
#include <ctime>
#include <cstdlib>

enum Flags {
    EATING      = 1 << 0,
    HUNGRY      = 1 << 1,
    RIGHT_FORK  = 1 << 2,
    RIGHT_TURN  = 1 << 3,
    LEFT_FORK   = 1 << 4,
    LEFT_TURN   = 1 << 5
};


#define MAX 10;


unsigned char initState(int rank){
    unsigned char state = 0;
    
    state |= RIGHT_TURN;
    state |= (rand() % 2) ? HUNGRY : state;

    return state;
}

void update(unsigned char* myState, int rightNeighbour, int leftNeighbour){
    MPI_Send(&myState, 1, MPI_UNSIGNED_CHAR, rightNeighbour, MPI_ANY_TAG, MPI_COMM_WORLD);
    MPI_Send(&myState, 1, MPI_UNSIGNED_CHAR, leftNeighbour, MPI_ANY_TAG, MPI_COMM_WORLD);
}

void check(unsigned char* myState, unsigned char* rightState, unsigned char* leftState, int rightNeighbour, int leftNeighbour){
        int flag;

        MPI_Iprobe(leftNeighbour, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
        if (flag) MPI_Recv(&leftState, 1, MPI_UNSIGNED_CHAR, leftNeighbour, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Iprobe(rightNeighbour, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
        if (flag) MPI_Recv(&rightState, 1, MPI_UNSIGNED_CHAR, rightNeighbour, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

void worker(int rank, int size){
    int rightNeighbour = (rank + 1) % size;
    int leftNeighbour = (size + (rank - 1)) % size;
    
    unsigned char leftState = initState(leftNeighbour);
    unsigned char myState = initState(rank);
    unsigned char rightState = initState(rightNeighbour);
    
    int timer = 0;

    while (true) {
        check(&myState, &rightState, &leftState, rightNeighbour, leftNeighbour);
        if (timer > 0) {
            timer--;
            update(&myState, rightNeighbour, leftNeighbour); continue;
        }
        check(&myState, &rightState, &leftState, rightNeighbour, leftNeighbour);
        if (myState & EATING) {
            timer = rand() % MAX;
            myState ^= (EATING | RIGHT_FORK | LEFT_FORK);
            update(&myState, rightNeighbour, leftNeighbour); continue;
        }
        check(&myState, &rightState, &leftState, rightNeighbour, leftNeighbour);
        if (not (myState & HUNGRY)) {
            myState |= HUNGRY;
            update(&myState, rightNeighbour, leftNeighbour); continue;
        }


        check(&myState, &rightState, &leftState, rightNeighbour, leftNeighbour);
        if ((myState & ~RIGHT_FORK) & (rightState & (~EATING | ~HUNGRY))){
            myState ^= RIGHT_FORK;
            update(&myState, rightNeighbour, leftNeighbour); continue;
        }
        check(&myState, &rightState, &leftState, rightNeighbour, leftNeighbour);
        if ((myState & (~RIGHT_FORK | RIGHT_TURN)) &  (rightState & (~EATING | HUNGRY | ~RIGHT_FORK))){
            myState ^= (RIGHT_FORK | RIGHT_TURN);
            update(&myState, rightNeighbour, leftNeighbour); continue;
        }
        check(&myState, &rightState, &leftState, rightNeighbour, leftNeighbour);
        if ((myState & (~RIGHT_FORK | ~RIGHT_TURN)) &  (rightState & (~EATING | HUNGRY | ~RIGHT_FORK))){
            myState ^= (RIGHT_FORK | RIGHT_TURN);
            update(&myState, rightNeighbour, leftNeighbour); continue;
        }
        check(&myState, &rightState, &leftState, rightNeighbour, leftNeighbour);
        if ((myState & (~RIGHT_FORK)) &  (rightState & RIGHT_FORK)){
            update(&myState, rightNeighbour, leftNeighbour); continue;
        }
        


        check(&myState, &rightState, &leftState, rightNeighbour, leftNeighbour);
        if ((myState & ~LEFT_FORK) & (leftState & (~EATING | ~HUNGRY))){
            myState ^= LEFT_FORK;
            update(&myState, rightNeighbour, leftNeighbour); continue;
        }
        check(&myState, &rightState, &leftState, rightNeighbour, leftNeighbour);
        if ((myState & (~LEFT_FORK | LEFT_TURN)) &  (leftState & (~EATING | HUNGRY | ~LEFT_FORK))){
            myState ^= (LEFT_FORK | LEFT_TURN);
            update(&myState, rightNeighbour, leftNeighbour); continue;
        }
        check(&myState, &rightState, &leftState, rightNeighbour, leftNeighbour);
        if ((myState & (~LEFT_FORK | ~LEFT_TURN)) &  (leftState & (~EATING | HUNGRY | ~LEFT_FORK))){
            myState ^= (LEFT_FORK | LEFT_TURN);
            update(&myState, rightNeighbour, leftNeighbour); continue;
        }
        check(&myState, &rightState, &leftState, rightNeighbour, leftNeighbour);
        if ((myState & (~LEFT_FORK)) &  (leftState & LEFT_FORK)){
            update(&myState, rightNeighbour, leftNeighbour); continue;
        }

        
        timer = rand() % MAX;
        myState ^= EATING;
        myState ^= HUNGRY;
        update(&myState, rightNeighbour, leftNeighbour);
        continue;
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    srand(time(NULL) + rank);

    worker(rank, size);

    MPI_Finalize();
    
    return 0;
}