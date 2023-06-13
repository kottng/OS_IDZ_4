#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>


#define SHARED_MEMORY_KEY 12345
#define SHARED_MEMORY_SIZE sizeof(int)
#define FIFO_FILE "MYFIFO"
#define BUFFER_SIZE 256


int main(int argc, char *argv[]) {
    int clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];
    pid_t childPid1, childPid2;
    int shmId;
    int *sharedValue;

    shmId = shmget(SHARED_MEMORY_KEY, SHARED_MEMORY_SIZE, IPC_CREAT | 0666);
    if (shmId == -1) {
        perror("Failed to create shared memory");
        exit(1);
    }

    sharedValue = (int *) shmat(shmId, NULL, 0);
    if (sharedValue == (int *) -1) {
        perror("Failed to attach shared memory");
        exit(1);
    }
    *sharedValue = 1;

    // проверка корректности параметров
    if (argc != 5) {
        printf("Usage: <%s> <server ip> <server port> <team id> <num of areas>\n", argv[0]);
        exit(1);
    }

    int id = atoi(argv[3]);
    int areas_number = atoi(argv[4]);
    // Создание клиентского сокета
    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket < 0) {
        perror("Error while creating socket\n");
        exit(1);
    }

    // Настройка серверного адреса
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &serverAddr.sin_addr) <= 0) {
        perror("Error while converting IP-address\n");
        exit(1);
    }

    // Подключение к серверу
    sprintf(buffer, "HI");
    sendto(clientSocket, &buffer, sizeof(buffer), 0, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
    printf("Connected to the server\n");

    // Получение массива участков от сервера
    int lands[2 * areas_number];
    struct sockaddr_in s_response;
    socklen_t len_of_addr = sizeof(s_response);
    ssize_t bytesReceived = recvfrom(clientSocket, lands, sizeof(lands), 0, (struct sockaddr *) &s_response,
                                     &len_of_addr);
    if (bytesReceived < 0) {
        perror("Error while receiving an array of lands from server");
        exit(1);
    }
    printf("An array of lands received:\n");
    for (int i = 0; i < 2 * areas_number - 1; i += 2) {
        printf("land %d, status: %d\n", lands[i], lands[i + 1]);
    }
    // здесь начинается обработка
    childPid1 = fork();
    if (childPid1 < 0) {
        perror("Error while creating a new process");
        exit(1);
    } else if (childPid1 == 0) {
        int i = 0;
        while (i < 2 * areas_number - 1 && *sharedValue) {
            unsigned int seed = (unsigned int) (getpid() + i);
            srand(seed);
            sleep(rand() % 16 + 4); // Пауза для осмотра участка
            if (lands[i + 1] == 1 && *sharedValue) {
                sprintf(buffer, "Treasure is found by team %d", id - 1);
                printf("%s\n", buffer);
                // Найден клад, отправка сообщения на сервер
                if (sendto(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &serverAddr,
                           sizeof(serverAddr)) < 0) {
                    perror("Error wwhile sending a message to the server");
                    exit(1);
                }
                *sharedValue = 0;
                break;
            }
            printf("team %d did not find anything\n", id);
            i += 2;
        }
        *sharedValue = 0;
        exit(1);
    }
    childPid2 = fork();

    if (childPid2 < 0) {
        perror("Error while creating new process\n");
        exit(1);
    } else if (childPid2 == 0) {
        int bytesRead = -1;
        memset(buffer, 0, sizeof(buffer));
        do {
            bytesRead = recvfrom(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &s_response,
                                 &len_of_addr);
            if (bytesRead >= 0) {
                *sharedValue = 0;
            }

            sleep(2);
        } while (bytesRead < 0 && *sharedValue);

        sprintf(buffer, "team %d did not find anything\n", id - 1);
        // Найден клад, отправка сообщения на сервер
        if (sendto(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &serverAddr,
                   sizeof(serverAddr)) < 0) {
            perror("Error wwhile sending a message to the server");
            exit(1);
        }
        exit(1);
    }

    for (int i = 0; i < 2; ++i) {
        int status;
        waitpid(-1, &status, 0);
    }
    printf("Client is ended\n");
    close(clientSocket);
    return 0;
}
