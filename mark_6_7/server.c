#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>


#define MAX_CLIENTS 10
#define BUFFER_SIZE 256

int numClients = 0;

int main(int argc, char *argv[]) {
    int serverSocket;
    struct sockaddr_in serverAddr;
    pid_t childPid;
    // проверка корректности параметров
    if (argc != 4) {
        printf("Usage: <%s> <port> <areas number per team> <amount number of teams>\n", argv[0]);
        exit(1);
    }
    int areas_number = atoi(argv[2]);
    int clients_number = atoi(argv[3]);

    // Создание серверного сокета
    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket < 0) {
        perror("Ошибка при создании сокета");
        exit(1);
    }

    struct sockaddr_in clientSockets[clients_number];
    struct sockaddr_in outers;
    // Настройка серверного адреса
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[1]));
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int server_len_addr = sizeof(serverAddr);

    // Привязка сокета к серверному адресу
    if (bind(serverSocket, (struct sockaddr *) &serverAddr, server_len_addr) < 0) {
        perror("Error in binding server socket");
        exit(1);
    }

    printf("Server is started and waiting for clients...\n");


    // подключение outer:
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    ssize_t recv_res = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &serverAddr,
                                (socklen_t *) &server_len_addr);
    // Принятие входящего подключения
    if (recv_res < 0) {
        printf("Error in accept() function for outer\n");
        exit(1);
    }
    memcpy(&outers, &serverAddr, sizeof(serverAddr));

    // отправка сообщений к outer с количеством area_number и приветствием
    if (sendto(serverSocket, &areas_number, sizeof(areas_number), 0,
               (struct sockaddr *) &outers, sizeof(struct sockaddr_in)) < 0) {
        perror("Error while sending to outer");
        exit(1);
    }

    if (sendto(serverSocket, "HI FROM SERVER", sizeof("HI FROM SERVER"), 0,
               (struct sockaddr *) &outers, sizeof(struct sockaddr_in)) < 0) {
        perror("Error while sending to outer");
        exit(1);
    }

    // подключение клиентов
    for (int i = 0; i < clients_number; ++i) {
        memset(buffer, 0, sizeof(buffer));
        recv_res = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &serverAddr,
                            (socklen_t *) &server_len_addr);
        // Принятие входящего подключения
        if (recv_res < 0) {
            printf("Error in accept() function for client %d\n", i);
            exit(1);
        }
        memcpy(&clientSockets[i], &serverAddr, sizeof(serverAddr));
        printf("Team %d connected\n", i);
    }

    for (int i = 0; i < clients_number; ++i) {
        // создание процесса обработки для каждого клиента
        childPid = fork();
        if (childPid < 0) {
            perror("Ошибка при создании нового процесса");
            exit(1);
        } else if (childPid == 0) {
            unsigned int seed = (unsigned int) (getpid() + i);
            srand(seed);
            int lands[2 * areas_number];
            // i's это элемент содержащий номер участка, i + 1's элемент содержащий статус этого участка:
            // 0 - клада нет / 1 - клад есть
            int j;
            int counter = 0;
            for (j = 0; j < 2 * areas_number - 1; j += 2) {
                lands[j] = counter;
                lands[j + 1] = (rand()) % 2;
//                lands[j + 1] = 0;
                ++counter;
            }
            struct sockaddr_in c_response;
            socklen_t len_of_client_addr = sizeof(c_response);
            if (sendto(serverSocket, lands, sizeof(lands), 0, (struct sockaddr *) &clientSockets[i],
                       sizeof(struct sockaddr_in)) < 0) {
                perror("Error while sending lands to the outer");
                exit(1);
            }
            char msg[BUFFER_SIZE];
            sprintf(msg, "Lands were deployed to the client");
            if (sendto(serverSocket, msg, sizeof(msg), 0, (struct sockaddr *) &outers,
                       sizeof(struct sockaddr_in)) < 0) {
                perror("Error while sending lands to the client");
                exit(1);
            }
            if (sendto(serverSocket, lands, sizeof(lands), 0, (struct sockaddr *) &outers,
                       sizeof(struct sockaddr_in)) < 0) {
                perror("Error while sending an array of lands to the outers");
                exit(1);
            }
            printf("Sending arrays of lands to clients is ended\n");
            int bytesRead = -1;
            memset(buffer, 0, sizeof(buffer));
            char buffer_2[BUFFER_SIZE];
            // прослушивание сообщений от клиентов, кто первый отправит сообщение серверу с победным сообщением,
            // тот и будет считаться победителем
            int counter_of_ended = 0;
            int is_treasure_found = 0;
            int flague_to_exit = 1;
            do {
                memset(buffer, 0, sizeof(buffer));
                bytesRead = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &c_response,
                                     (socklen_t *) &len_of_client_addr);
                strncpy(buffer_2, buffer, 25);
                printf("buffer 2 is %s\n", buffer_2);
                if (strcmp(buffer_2, "Treasure is found by team") == 0) {
                    is_treasure_found = 1;
                    flague_to_exit = 0;
                } else {
                    if (counter_of_ended < 3) {
                        ++counter_of_ended;
                        printf("counter_of_ended %d\n", counter_of_ended);
                    } else {
                        flague_to_exit = 0;
                        printf("ALL IS REALLY WRONG\n");
                    }
                }
                sleep(2);
            } while (flague_to_exit);
            if (is_treasure_found) {
                // отправка сообщений с указанием о победе всем клиентам
                printf("%s\n", buffer);
                for (j = 0; j < clients_number; ++j) {
                    if (sendto(serverSocket, "IT IS DONE", sizeof("IT IS DONE"), 0,
                               (struct sockaddr *) &clientSockets[j], sizeof(struct sockaddr_in)) < 0) {
                        perror("Ошибка при отправке сообщения на сервер");
                        exit(1);
                    }
                }
                if (sendto(serverSocket, buffer, sizeof(buffer), 0,
                           (struct sockaddr *) &outers, sizeof(struct sockaddr_in)) < 0) {
                    perror("Error while sending message to outers");

                    exit(1);
                }
                if (sendto(serverSocket, "break", sizeof("break"), 0,
                           (struct sockaddr *) &outers, sizeof(struct sockaddr_in)) < 0) {
                    perror("Ошибка при отправке сообщения на сервер");
                    exit(1);
                }
                kill(0, SIGTERM);
            } else {
                sprintf(buffer, "Teams found nothing");
                if (sendto(serverSocket, buffer, sizeof(buffer), 0,
                           (struct sockaddr *) &outers, sizeof(struct sockaddr_in)) < 0) {
                    perror("Error while sending message to outers");

                    exit(1);
                }
                if (sendto(serverSocket, "break", sizeof("break"), 0,
                           (struct sockaddr *) &outers, sizeof(struct sockaddr_in)) < 0) {
                    perror("Ошибка при отправке сообщения на сервер");
                    exit(1);
                }
                kill(0, SIGTERM);
            }
        }
    }
    for (int i = 0; i < clients_number; ++i) {
        int status;
        waitpid(-1, &status, 0);
    }
    close(serverSocket);
    return 0;
}
