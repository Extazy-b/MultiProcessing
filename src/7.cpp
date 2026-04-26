#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>
#include <queue>
#include <stack>
#include <chrono>
#include <thread>
#include <mpi.h>
#include "CSLL.h"
#include "ISLL.h"

#define READING_HELLO 0
#define WRITING_HELLO 1
#define SERVER_OK 2
#define WRITING_DATA 3
#define WRITING_DATA_STOP 4
#define SERVER_DATA 5

#define DATA_TYPE MPI_INT
#define DATA_SIZE 1

#define WRITER_TYPE_ID 0
#define READER_TYPE_ID 1

#define SERVER_ID 0
#define OBSERVER_ID 1



void observer() {

}


void server() {
    ISLL storage = ISLL();
    CSLL activeReaders = CSLL();

    MPI_Status status, tmpStatus;
    int msgFlag;

    std::queue<int> writersWaitingList;
    int session = -1;

    int* tmp = new int();

    while (true){
        if (!activeReaders.empty()) {
            if (activeReaders.current()->current == storage.end()){
                activeReaders.removeCurrent();
            }
            else
            {
            MPI_Send(&activeReaders.current()->current->data, DATA_SIZE, DATA_TYPE, activeReaders.current()->rank, SERVER_DATA, MPI_COMM_WORLD);
            activeReaders.current()->current = activeReaders.current()->current->next;
            activeReaders.advance();
            }
        }

        if ((session == -1) and (!writersWaitingList.empty())) {
            session = writersWaitingList.front();
            writersWaitingList.pop();
            MPI_Send(tmp, DATA_SIZE, DATA_TYPE, session, SERVER_OK, MPI_COMM_WORLD);
        }
        
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &msgFlag, &tmpStatus);
        if (msgFlag) {
            MPI_Recv(tmp, DATA_SIZE, DATA_TYPE, tmpStatus.MPI_SOURCE, tmpStatus.MPI_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG == READING_HELLO) {
                activeReaders.add(status.MPI_SOURCE, storage.begin());
                continue;
            }

            if (status.MPI_TAG == WRITING_HELLO) {
                if ((session == -1) and (writersWaitingList.empty())) {
                    session = status.MPI_SOURCE;
                    MPI_Send(tmp, DATA_SIZE, DATA_TYPE, session, SERVER_OK, MPI_COMM_WORLD);
                    continue;
                } else
                {
                    writersWaitingList.push(status.MPI_SOURCE);
                    continue;
                }                
            }

            if (status.MPI_TAG == WRITING_DATA) {
                if (status.MPI_SOURCE != session) continue;
                storage.push(*tmp);
                continue;
            }

            if (status.MPI_TAG == WRITING_DATA_STOP){ session = -1; continue;}
        }
    }

    delete tmp;
}

void writer(int id) {

}

void reader(int id) {
    
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int readerCount = 4;  
    srand(time(NULL) + rank);

    if (rank == SERVER_ID) {
        server();
    } else if (rank == OBSERVER_ID) {
        observer();
    } else if (rank < readerCount + 2) {       
        writer(rank);
    } else {
        reader(rank);
    }

    MPI_Finalize();
    return 0;
}
