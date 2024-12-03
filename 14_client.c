#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];
    socklen_t server_addr_size = sizeof(server_addr);

    // Создание UDP-сокета
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Ошибка создания сокета");
        exit(1);
    }

    // Настройка адреса сервера
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // IP-адрес сервера
    server_addr.sin_port = htons(12345);

    printf("Подключение к серверу. Для выхода введите 'exit'.\n");

    while (1) {
        // Ввод сообщения пользователем
        printf("Ваше сообщение: ");
        fgets(buffer, BUF_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';  // Удаление символа новой строки

        // Отправка сообщения серверу
        sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, server_addr_size);

        if (strcmp(buffer, "exit") == 0) {
            printf("Выход из чата.\n");
            break;
        }

        // Получение ответа от сервера
        int recv_len = recvfrom(sock, buffer, BUF_SIZE, 0, (struct sockaddr*)&server_addr, &server_addr_size);
        if (recv_len == -1) {
            perror("Ошибка получения данных");
            break;
        }
        buffer[recv_len] = '\0';  // Завершение строки

        printf("Ответ сервера: %s\n", buffer);

        if (strcmp(buffer, "exit") == 0) {
            printf("Сервер завершил сеанс.\n");
            break;
        }
    }

    close(sock);
    return 0;
}
