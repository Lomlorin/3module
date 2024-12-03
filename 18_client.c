#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>  // Для gethostbyname

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buff[1024];

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    // Используем gethostbyname для разрешения доменного имени или IP-адреса
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    }

    // Цикл для отправки нескольких сообщений
    while (1) {
        printf("Enter message: ");
        bzero(buff, sizeof(buff));
        fgets(buff, sizeof(buff) - 1, stdin);
        
        // Проверка на выход
        if (strncmp(buff, "quit", 4) == 0) {
            send(sockfd, buff, strlen(buff), 0);
            break;
        }

        // Отправка сообщения серверу
        n = send(sockfd, buff, strlen(buff), 0);
        if (n < 0) {
            error("ERROR writing to socket");
        }

        // Получение ответа от сервера
        bzero(buff, sizeof(buff));
        n = recv(sockfd, buff, sizeof(buff) - 1, 0);
        if (n < 0) {
            error("ERROR reading from socket");
        }
        printf("Server response: %s\n", buff);
    }

    close(sockfd);
    return 0;
}
