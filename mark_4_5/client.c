#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define BUFFER_SIZE 256


int main(int argc, char *argv[]) {
    int clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];
    int recvMsgSize;
    pid_t childPid1, childPid2;

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
        perror("Ошибка при создании сокета");
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
        perror("Ошибка при получении массива участков от сервера");
        exit(1);
    }


    printf("Получен массив участков:\n");
    for (int i = 0; i < 2 * areas_number - 1; i += 2) {
        printf("Участок %d, Статус: %d\n", lands[i], lands[i + 1]);
    }

    // здесь начинается обработка
    childPid1 = fork();
    if (childPid1 < 0) {
        perror("Ошибка при создании нового процесса");
        exit(1);
    } else if (childPid1 == 0) {
        int i = 0;
        while (i < 2 * areas_number - 1) {
            unsigned int seed = (unsigned int) (getpid() + i);
            srand(seed);
            sleep(rand() % 16 + 4); // Пауза для осмотра участка
            if (lands[i + 1] == 1) {
                printf("Treasure is find by team %d\n", id);
                // Найден клад, отправка сообщения на сервер
                if (sendto(clientSocket, "IT IS DONE", sizeof("IT IS DONE"), 0, (struct sockaddr *) &serverAddr,
                           sizeof(serverAddr)) < 0) {
                    perror("Ошибка при отправке сообщения на сервер");
                    exit(1);
                }
                break;
            }
            printf("team %d did not find anything\n", id);
            i += 2;
        }
        kill(0, SIGTERM);
        exit(1);
    }
    childPid2 = fork();
    if (childPid2 < 0) {
        perror("Ошибка при создании нового процесса");
        exit(1);
    } else if (childPid2 == 0) {
        int bytesRead = -1;
        memset(buffer, 0, sizeof(buffer));
        do {
            bytesRead = recvfrom(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &s_response,
                                 &len_of_addr);
            sleep(2);
        } while (bytesRead < 0);
        printf("%s\n", buffer);
        kill(0, SIGTERM);
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
