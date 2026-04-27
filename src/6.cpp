#include <cstdio>
#include <cstdlib>
#include <string>
#include <queue>
#include <chrono>
#include <thread>
#include <mpi.h>


#define MSG_PRODUCER_HELLO 0
#define MSG_CONSUMER_HELLO 1
#define MSG_SERVER_HELLO   2
#define MSG_PRODUCER_SEND  3
#define MSG_LOGGING  4


#define SERVER_ID 0
#define OBSERVER_ID 1 


#define PRODUCER_TYPE 0
#define CONSUMER_TYPE 1


const int storageSize = 10;
const int producerCount = 3;


struct logNote{
    int isSuccseed;
    int type;
    int id;
    int storageUsage;
};

void sendLog(int isSuccseed, int type, int id, int storageUsage) {
    logNote data = {isSuccseed, type, id, storageUsage};
    MPI_Send(&data, 5, MPI_INT, OBSERVER_ID, MSG_LOGGING, MPI_COMM_WORLD);
}


void getData(std::queue<int>& storage, int& data, MPI_Status& status, int source){
    char zero = 0;
    MPI_Send(&zero, 1, MPI_CHAR, source, MSG_SERVER_HELLO, MPI_COMM_WORLD);
    MPI_Recv(&data, 1, MPI_INT, source, MSG_PRODUCER_SEND, MPI_COMM_WORLD, &status);
    storage.push(data);
    
    sendLog(0, PRODUCER_TYPE, source, storage.size());
}


void sendData(std::queue<int>& storage, int dest){
    MPI_Send(&storage.front(), 1, MPI_INT, dest, MSG_SERVER_HELLO, MPI_COMM_WORLD);                
    storage.pop();

    sendLog(0, CONSUMER_TYPE, dest, storage.size());
}

void server() {
        std::queue<int> storage;
        std::queue<int> producers;
        std::queue<int> consumers;
        MPI_Status status, probe_status;
        int data, source, flag;

        while (true){
            while (!consumers.empty() && !storage.empty()) {
                sendData(storage, consumers.front());
                consumers.pop();
            }

            while (!producers.empty() && storage.size() < storageSize) {
                getData(storage, data, status, producers.front());
                producers.pop();
            }

            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &probe_status);
            if (!flag) {
                continue;
            }

            MPI_Recv(&data, 1, MPI_INT, probe_status.MPI_SOURCE, probe_status.MPI_TAG, MPI_COMM_WORLD, &status);

            if (status.MPI_SOURCE == SERVER_ID) {continue;}
            if (status.MPI_SOURCE == OBSERVER_ID) {continue;}

            if (status.MPI_TAG == MSG_PRODUCER_HELLO) {
                source = status.MPI_SOURCE;
                if (storage.size() == storageSize) {
                    producers.push(source); 
                    sendLog(1, PRODUCER_TYPE, source, storage.size());
                    continue;
                };

                getData(storage, data, status, source);
                continue;
            }

            if (status.MPI_TAG == MSG_CONSUMER_HELLO) {
                if (storage.empty()) {
                    consumers.push(status.MPI_SOURCE); 
                    sendLog(1, CONSUMER_TYPE, status.MPI_SOURCE, storage.size());
                    continue;
                };

                sendData(storage, status.MPI_SOURCE);
                continue;
            }
        };
}

void observer() {
    logNote note;

    std::cout << "===== Протокол работы (буфер " << storageSize << " элементов) =====\n";

    while (true) {
        MPI_Recv(&note, 5, MPI_INT, MPI_ANY_SOURCE, MSG_LOGGING, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        std::string who = (note.type == PRODUCER_TYPE) ? "Производитель" : "Потребитель";
        std::string action;
        if (note.type == PRODUCER_TYPE) {
            action = note.isSuccseed ? "не смог записать в хранилище"
                                     : "успешно записал в хранилище";
        } else {
            action = note.isSuccseed ? "не смог прочесть хранилище"
                                     : "успешно прочёл хранилище";
        }

        std::cout << who << " " << note.id << " "
                  << action << " (заполнено " << note.storageUsage
                  << "/" << storageSize << ")" << std::endl;
    }
}

void producer(int id) {
    char zero = 0;
    while (true){
        int workTime = rand() % 10 + 3;
        std::this_thread::sleep_for(std::chrono::seconds(workTime));
        MPI_Send(&id, 1, MPI_INT, SERVER_ID, MSG_PRODUCER_HELLO, MPI_COMM_WORLD);
        MPI_Recv(&zero, 1, MPI_CHAR, SERVER_ID, MSG_SERVER_HELLO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(&id, 1, MPI_INT, SERVER_ID, MSG_PRODUCER_SEND, MPI_COMM_WORLD);
    }
}

void consumer(int id) {
    char zero = 0;
    int data = 0;
    while (true){
        int workTime = rand() % 10 + 3;
        MPI_Send(&id, 1, MPI_INT, SERVER_ID, MSG_CONSUMER_HELLO, MPI_COMM_WORLD);
        MPI_Recv(&data, 1, MPI_INT, SERVER_ID, MSG_SERVER_HELLO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::this_thread::sleep_for(std::chrono::seconds(workTime));
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);


    int consumerCount = size - 2 - producerCount;
    srand(time(NULL) + rank);
    if (rank == SERVER_ID) {
        server();
    } else if (rank == OBSERVER_ID) {
        observer();
    } else if (rank < producerCount + 2) {       
        producer(rank);
    } else {
        consumer(rank);
    }

    MPI_Finalize();
    return 0;
}
