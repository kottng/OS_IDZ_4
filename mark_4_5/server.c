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
#include <sys/ipc.h>
#include <sys/shm.h>


#define MAX_CLIENTS 10
#define BUFFER_SIZE 256
#define SHARED_MEMORY_KEY 12345
#define SHARED_MEMORY_SIZE sizeof(int)

int numClients = 0;

int main(int argc, char *argv[]) {
    int shmId;
    int *counter_of_ended;
    int serverSocket;
    struct sockaddr_in serverAddr;
    pid_t childPid;
    // проверка корректности параметров
    if (argc != 4) {
        printf("Usage: <%s> <port> <areas number> <clients number>\n", argv[0]);
        exit(1);
    }
    int areas_number = atoi(argv[2]);
    int clients_number = atoi(argv[3]);
    shmId = shmget(SHARED_MEMORY_KEY, SHARED_MEMORY_SIZE, IPC_CREAT | 0666);
    if (shmId == -1) {
        perror("Failed to create shared memory");
        exit(1);
    }

    counter_of_ended = (int *) shmat(shmId, NULL, 0);
    if (counter_of_ended == (int *) -1) {
        perror("Failed to attach shared memory");
        exit(1);
    }
    *counter_of_ended = 0;

    // Создание серверного сокета
    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket < 0) {
        perror("Ошибка при создании сокета");
        exit(1);
    }

    struct sockaddr_in clientSockets[clients_number];
    // Настройка серверного адреса
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[1]));
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int server_len_addr = sizeof(serverAddr);

    // Привязка сокета к серверному адресу
    if (bind(serverSocket, (struct sockaddr *) &serverAddr, server_len_addr) < 0) {
        perror("Ошибка при привязке сокета");
        exit(1);
    }

    printf("Сервер запущен и ожидает подключений...\n");

    for (int i = 0; i < clients_number; ++i) {
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        ssize_t recv_res = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &serverAddr,
                                    (socklen_t *) &server_len_addr);
        // Принятие входящего подключения
        if (recv_res < 0) {
            printf("Error in accept() function for client %d\n", i);
            exit(1);
        }
        memcpy(&clientSockets[i], &serverAddr, sizeof(serverAddr));
        printf("Client %d connected\n", i);
    }
    for (int i = 0; i < clients_number; ++i) {
        childPid = fork();
        if (childPid < 0) {
            perror("Ошибка при создании нового процесса");
            exit(1);
        } else if (childPid == 0) {
            unsigned int seed = (unsigned int) (getpid() + i);
            srand(seed);
            int lands[2 * areas_number];
            // i's elem is number of area, i + 1's elem is status of this area
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
                perror("Ошибка при отправке массива участков клиенту");
                exit(1);
            }
            printf("Отправка участков для клиента %d завершена\n", i);
            char buffer[BUFFER_SIZE];
            int bytesRead = -1;
            memset(buffer, 0, sizeof(buffer));

            int is_treasure_found = 0;
            int flague_to_exit = 1;
            do {
                memset(buffer, 0, sizeof(buffer));
                bytesRead = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (struct sockaddr *) &c_response,
                                     (socklen_t *) &len_of_client_addr);
                if (strcmp(buffer, "IT IS DONE") == 0) {
                    is_treasure_found = 1;
                    flague_to_exit = 0;
                } else {
                    if (*counter_of_ended < clients_number) {
                        ++(*counter_of_ended);
                    } else {
                        flague_to_exit = 0;
                    }
                }
                sleep(2);
            } while (flague_to_exit);
            if (is_treasure_found) {
                printf("%s\n", buffer);
                for (j = 0; j < clients_number; ++j) {
                    if (sendto(serverSocket, "IT IS DONE", sizeof("IT IS DONE"), 0,
                               (struct sockaddr *) &clientSockets[j], sizeof(struct sockaddr_in)) < 0) {
                        perror("Ошибка при отправке сообщения на сервер");
                        exit(1);
                    }
                }
                kill(0, SIGTERM);
            } else {
                printf("Treasure was not found\n");
                kill(0, SIGTERM);
            }
            exit(1);
        }
    }
    for (int i = 0; i < clients_number; ++i) {
        int status;
        waitpid(-1, &status, 0);
    }
    close(serverSocket);
    return 0;
}
