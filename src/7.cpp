#include <iostream>
#include <cstdlib>
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
#define SERVER_DATA_STOP 6

#define DATA_TYPE MPI_INT
#define DATA_SIZE 1

#define WRITER_TYPE_ID 0
#define READER_TYPE_ID 1

#define SERVER_ID 0
#define OBSERVER_ID 1

int signalZero = 0;
int tmpData = 0;

#define MSG_LOGGING 7

struct LogNote {
    int eventType; 
    int processRank;
    int data;
    int storageSize;
};

void sendLog(int eventType, int rank, int data, int storageSize) {
    LogNote note = {eventType, rank, data, storageSize};
    MPI_Send(&note, 4, MPI_INT, OBSERVER_ID, MSG_LOGGING, MPI_COMM_WORLD);
}

void observer() {
    LogNote note;
    MPI_Status status;

    std::cout << "===== Протокол работы (Читатели-Писатели) =====\n";

    while (true) {
        MPI_Recv(&note, 4, MPI_INT, MPI_ANY_SOURCE, MSG_LOGGING, MPI_COMM_WORLD, &status);

        std::string who = (note.eventType <= 1) ? "Писатель" : "Читатель";
        std::string action;

        switch (note.eventType) {
            case 0: // писатель добавил запись
                action = "добавил запись " + std::to_string(note.data);
                break;
            case 1: // писатель завершил сессию
                action = "завершил запись";
                break;
            case 2: // читатель начал чтение
                action = "начал чтение";
                break;
            case 3: // читатель получил запись
                action = "получил запись " + std::to_string(note.data);
                break;
            case 4: // читатель завершил чтение
                action = "завершил чтение";
                break;
            default:
                action = "неизвестное событие";
        }

        std::cout << who << " " << note.processRank << " "
                  << action << " (записей в хранилище: " << note.storageSize << ")"
                  << std::endl;
    }
}

void mySleep() {
    std::this_thread::sleep_for(std::chrono::seconds(rand() % 10));
}

void server() {
    ISLL storage = ISLL();
    CSLL activeReaders = CSLL();

    MPI_Status status, tmpStatus;
    int msgFlag;

    std::queue<int> writersWaitingList = std::queue<int>();
    int session = -1;

    while (true){
        if (!activeReaders.empty()) {
            if (activeReaders.current()->current == storage.end()){
                sendLog(4, activeReaders.current()->rank, 0, storage.size());
                MPI_Send(&signalZero, 1, MPI_INT, activeReaders.current()->rank, SERVER_DATA_STOP, MPI_COMM_WORLD);                
                activeReaders.removeCurrent();
            }
            else
            {
            sendLog(3, activeReaders.current()->rank, activeReaders.current()->current->data, storage.size());
            MPI_Send(&activeReaders.current()->current->data, DATA_SIZE, DATA_TYPE, activeReaders.current()->rank, SERVER_DATA, MPI_COMM_WORLD);
            activeReaders.current()->current = activeReaders.current()->current->next;
            activeReaders.advance();
            }
        }

        if ((session == -1) and (!writersWaitingList.empty())) {
            session = writersWaitingList.front();
            writersWaitingList.pop();
            MPI_Send(&signalZero, 1, MPI_INT, session, SERVER_OK, MPI_COMM_WORLD);
        }
        
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &msgFlag, &tmpStatus);
        if (msgFlag) {
        if (tmpStatus.MPI_SOURCE == SERVER_ID || tmpStatus.MPI_SOURCE == OBSERVER_ID) {
            MPI_Recv(&signalZero, 1, MPI_INT, tmpStatus.MPI_SOURCE, tmpStatus.MPI_TAG, MPI_COMM_WORLD, &status);
            continue;
        }
        switch (tmpStatus.MPI_TAG) {
            case READING_HELLO:
                MPI_Recv(&signalZero, 1, MPI_INT, tmpStatus.MPI_SOURCE, tmpStatus.MPI_TAG, MPI_COMM_WORLD, &status);
                activeReaders.add(status.MPI_SOURCE, storage.begin());
                sendLog(2, status.MPI_SOURCE, 0, storage.size());
                break;

            case WRITING_HELLO:
                MPI_Recv(&signalZero, 1, MPI_INT, tmpStatus.MPI_SOURCE, tmpStatus.MPI_TAG, MPI_COMM_WORLD, &status);
                if (session == -1 && writersWaitingList.empty()) {
                    session = status.MPI_SOURCE;
                    MPI_Send(&signalZero, 1, MPI_INT, session, SERVER_OK, MPI_COMM_WORLD);
                } else {
                    writersWaitingList.push(status.MPI_SOURCE);
                }
                break;

            case WRITING_DATA:
                MPI_Recv(&tmpData, DATA_SIZE, DATA_TYPE, tmpStatus.MPI_SOURCE, tmpStatus.MPI_TAG, MPI_COMM_WORLD, &status);
                if (status.MPI_SOURCE == session) {
                    storage.push(tmpData);
                }
                sendLog(0, status.MPI_SOURCE, tmpData, storage.size());
                break;

            case WRITING_DATA_STOP:
                MPI_Recv(&tmpData, DATA_SIZE, DATA_TYPE, tmpStatus.MPI_SOURCE, tmpStatus.MPI_TAG, MPI_COMM_WORLD, &status);
                if (status.MPI_SOURCE == session) session = -1;
                sendLog(1, status.MPI_SOURCE, 0, storage.size());
                break;

            default:
                break;
        }
        }

    }
}

void writer() {
    int buffer_size = rand() % 100;    
    std::queue<int> buffer = std::queue<int>();
    bool session = false;
    while (true) {
        if (session) {
            if (!buffer.empty()){
            MPI_Send(&buffer.front(), DATA_SIZE, DATA_TYPE, SERVER_ID, WRITING_DATA, MPI_COMM_WORLD);
            buffer.pop();
            }
            else {
            MPI_Send(&signalZero, 1, MPI_INT, SERVER_ID, WRITING_DATA_STOP, MPI_COMM_WORLD);
            buffer_size = rand() % 100;
            session = !session;
            }
        }
        else {
        if (buffer.size() < buffer_size) {
            buffer.push(rand());
        }
        else {
            MPI_Send(&signalZero, 1, MPI_INT, SERVER_ID, WRITING_HELLO, MPI_COMM_WORLD);
            MPI_Recv(&signalZero, 1, MPI_INT, SERVER_ID, SERVER_OK, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            session = !session;
        }
        }
    }
}

void reader() {
    std::stack<int> data = std::stack<int>();
    MPI_Status status;
    bool session = false;

    while (true) {
        if (session) {
            MPI_Recv(&tmpData, DATA_SIZE, DATA_TYPE, SERVER_ID, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG == SERVER_DATA) {data.push(tmpData); continue;}
            if (status.MPI_TAG == SERVER_DATA_STOP) {session = !session; continue;}
        }
        else {
            data = {};
            MPI_Send(&signalZero, 1, MPI_INT, SERVER_ID, READING_HELLO, MPI_COMM_WORLD);
            session = !session;
        }
    }
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
        writer();
    } else {
        reader();
    }

    MPI_Finalize();
    return 0;
}
