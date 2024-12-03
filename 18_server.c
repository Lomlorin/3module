#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void dostuff(int sock) {
    int bytes_recv;
    char buff[1024];
    
    while (1) {
        // Чтение сообщения от клиента
        bzero(buff, 1024);
        bytes_recv = read(sock, buff, sizeof(buff) - 1);
        if (bytes_recv < 0) {
            error("ERROR reading from socket");
        }
        
        // Проверяем, не отправил ли клиент команду "quit"
        if (strncmp(buff, "quit", 4) == 0) {
            printf("Client requested to close the connection\n");
            break;
        }

        // Печатаем сообщение, отправленное клиентом
        printf("Message from client: %s\n", buff);

        // Ответ клиенту
        write(sock, "Message received", 16);
    }
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            error("ERROR on accept");
        }

        pid = fork();
        if (pid < 0) {
            error("ERROR on fork");
        }
        if (pid == 0) {
            close(sockfd);
            dostuff(newsockfd);
            close(newsockfd);
            exit(0);
        } else {
            close(newsockfd);
        }
    }

    close(sockfd);
    return 0;
}
