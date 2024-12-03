#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void dostuff(int sock) {
    char buffer[1024];
    int n;

    while (1) {
        bzero(buffer, sizeof(buffer));
        n = read(sock, buffer, sizeof(buffer) - 1);
        if (n <= 0) {
            printf("Клиент отключился.\n");
            close(sock);
            return;
        }

        // Обработка команды передачи файла
        if (strncmp(buffer, "файл ", 5) == 0) {
            char *filename = buffer + 5;
            filename[strcspn(filename, "\n")] = '\0'; // Убираем символ новой строки

            printf("Получение файла: %s\n", filename);
            FILE *file = fopen(filename, "wb");
            if (!file) {
                perror("Ошибка создания файла");
                continue;
            }

            while ((n = read(sock, buffer, sizeof(buffer))) > 0) {
                fwrite(buffer, 1, n, file);
                if (n < sizeof(buffer)) break; // Конец файла
            }

            fclose(file);
            printf("Файл %s сохранён.\n", filename);
            write(sock, "Файл получен и сохранён.\n", 26);
        } else {
            printf("Получено: %s", buffer);
            write(sock, "Команда обработана.\n", 21);
        }
    }
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
        fprintf(stderr, "Укажите порт.\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("Ошибка создания сокета");

    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("Ошибка связывания");

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0) error("Ошибка при подключении");

        if (fork() == 0) {
            close(sockfd);
            dostuff(newsockfd);
            exit(0);
        }
        close(newsockfd);
    }

    close(sockfd);
    return 0;
}
