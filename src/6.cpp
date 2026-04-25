#include <cstdio>
#include <cstdlib>
#include <cmath>
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


void sendLog(int whoType, int whoId, int isSuccsesed, int size) {
    int data = generateLogData(whoType, whoId, isSuccsesed, size);
    MPI_Send(&data, 1, MPI_INT, OBSERVER_ID, MSG_LOGGING, MPI_COMM_WORLD);
}


int generateLogData(int whoType, int whoId, int isSuccsesed, int size){
    return (whoId) + (whoType) * 2 + (isSuccsesed) * size * 2;
}


std::string parseLogData(int data, int size){
    int whoId = data % 2;
    int whoType = (int)(data / 2) % size;
    int isSuccsesed = data % (2 * size);
    std::string note = "";
    note += ((bool) !whoType ? "Производитель " : "Потребитель ");
    note += "с номером процесса" + std::to_string(whoId) + " ";
    note += ((bool) !isSuccsesed ? "успешно " : "неудачно ");
    note += ((bool) !whoType ? "записал в буффер" : "прочёл буффер");
    return note;
}


void getData(std::queue<int>& storage, int& data, MPI_Status& status, int source, int size){
    char zero = 0;
    MPI_Send(&zero, 1, MPI_CHAR, source, MSG_SERVER_HELLO, MPI_COMM_WORLD);
    MPI_Recv(&data, 1, MPI_INT, source, MSG_PRODUCER_SEND, MPI_COMM_WORLD, &status);
    storage.push(data);
    
    sendLog(0, source, 0, size);
}

void sendData(std::queue<int>& storage, int dest, int size){
    MPI_Send(&storage.front(), 1, MPI_INT, dest, MSG_SERVER_HELLO, MPI_COMM_WORLD);                
    storage.pop();

    sendLog(1, dest, 0, size);
}

void server(int storageSize, int size) {
        std::queue<int> storage;
        std::queue<int> producers;
        std::queue<int> consumers;
        MPI_Status status, probe_status;
        int data, source, flag;

        while (true){
            while (!consumers.empty() && !storage.empty()) {
                sendData(storage, consumers.front(), size);
                consumers.pop();
            }

            while (!producers.empty() && storage.size() < storageSize) {
                getData(storage, data, status, producers.front(), size);
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
                if (storage.size() == storageSize) {producers.push(source); continue;};
                getData(storage, data, status, source, size);
                continue;
            }
            if (status.MPI_TAG == MSG_CONSUMER_HELLO) {
                if (storage.empty()) {consumers.push(status.MPI_SOURCE); continue;}
                sendData(storage, status.MPI_SOURCE, size);
                continue;
            }
        };
}

void observer(int size) {
    
}

void producer(int id) {
    int waitTime; //TODO задать максимальное время ожидания ответа от сервера
    while (true){
        int workTime; // TODO задать случайное значение
        std::this_thread::sleep_for(std::chrono::seconds(workTime));
        int value; // TODO задать случайное значение
    }
    
}

void consumer(int id) {
    int waitTime; // задать максимальное время ожидания ответа от сервера
    int worktime; // задать рандомное определение
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int storageSize = 10;
    int manufactorCount = 3;

    int consumerCount = size - 2 - manufactorCount;

    if (rank == 0) {
        server(storageSize, size);
    } else if (rank == 1) {
        observer(size);
    } else if (rank < 2 + manufactorCount) {       
        producer(rank);
    } else {
        consumer(rank);
    }

    MPI_Finalize();
    return 0;
}
