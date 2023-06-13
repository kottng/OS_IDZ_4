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

    // проверка корректности параметров
    if (argc != 3) {
        printf("Usage: <%s> <server ip> <server port> \n", argv[0]);
        exit(1);
    }

    // Создание клиентского сокета
    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket < 0) {
        perror("Error while creating socket");
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
    struct sockaddr_in s_response;
    socklen_t len_of_addr = sizeof(s_response);
    // вводится количество участков:
    int areas_number = 0;
    if (recvfrom(clientSocket, &areas_number, sizeof(areas_number), 0, (struct sockaddr *) &s_response,
                 &len_of_addr) < 0) {
        perror("Error while receiving number of areas\n");
        exit(1);
    }
    int num_of_client = 0;
    int bytesRead;
    memset(buffer, 0, sizeof(buffer));
    bytesRead = recvfrom(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &s_response,
                         &len_of_addr);
    if (bytesRead < 0) {
        perror("Error while receiving message from client");
    }
    while (strcmp(buffer, "break") < 0) {
        printf("%s\n", buffer);
        if (strcmp(buffer, "Lands were deployed to the client") == 0) {
            // обработка случая, когда передаются массивы
            int lands[2 * areas_number];
            ssize_t bytesReceived = recvfrom(clientSocket, lands, sizeof(lands), 0, (struct sockaddr *) &s_response,
                                             &len_of_addr);
            if (bytesReceived < 0) {
                perror("Error while receiving lands from server");
                exit(1);
            }
            printf("array of lands received for team %d:\n", num_of_client);
            for (int i = 0; i < 2 * areas_number - 1; i += 2) {
                printf("land %d, status: %d\n", lands[i], lands[i + 1]);
            }
            ++num_of_client;
            memset(buffer, 0, sizeof(buffer));
        } else {
            // обработка случая, когда передаются стандартные сообщения
            memset(buffer, 0, sizeof(buffer));
            bytesRead = recvfrom(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &s_response,
                                 &len_of_addr);
            if (bytesRead < 0) {
                perror("Error while receiving message from client");
            }
        }
    }
    printf("Outer is ended\n");
    close(clientSocket);
    return 0;
}
