#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg) {
    perror(msg);
    exit(0);
}

void send_file(int sock, const char *filename) {
    FILE *file = fopen(filename, "rb");
    char buffer[1024];
    int n;

    if (!file) {
        perror("Ошибка открытия файла");
        return;
    }

    // Отправка файла по частям
    while ((n = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (write(sock, buffer, n) < 0) {
            perror("Ошибка отправки файла");
            break;
        }
    }

    fclose(file);
    printf("Файл %s отправлен.\n", filename);
}

int main(int argc, char *argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[1024];

    if (argc < 3) {
        fprintf(stderr, "Использование: %s <хост> <порт>\n", argv[0]);
        exit(0);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("Ошибка создания сокета");

    server = gethostbyname(argv[1]);
    if (!server) {
        fprintf(stderr, "Хост не найден\n");
        exit(0);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("Ошибка подключения");

    while (1) {
        printf("Введите команду: ");
        bzero(buffer, sizeof(buffer));
        fgets(buffer, sizeof(buffer) - 1, stdin);

        // Проверка на команду выхода
        if (strncmp(buffer, "выход", 5) == 0) {
            printf("Завершение работы.\n");
            break;
        }

        // Проверка на команду передачи файла
        if (strncmp(buffer, "файл ", 5) == 0) {
            char *filename = buffer + 5;
            filename[strcspn(filename, "\n")] = '\0'; // Убираем символ новой строки
            write(sockfd, buffer, strlen(buffer));   // Отправляем команду на сервер
            send_file(sockfd, filename);            // Отправляем сам файл
        } else {
            write(sockfd, buffer, strlen(buffer));
        }

        bzero(buffer, sizeof(buffer));
        n = read(sockfd, buffer, sizeof(buffer) - 1);
        if (n > 0) printf("Ответ сервера: %s", buffer);
    }

    close(sockfd);
    return 0;
}
