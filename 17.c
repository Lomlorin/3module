#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 65536

int main() {
    int sock;
    char buffer[BUF_SIZE];

    struct sockaddr_in source, dest;
    struct iphdr *iph;
    struct udphdr *udph;

    // Создание сокета
    sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sock < 0) {
        perror("Ошибка создания сокета");
        return 1;
    }

    while (1) {
        // Получение пакета
        ssize_t packet_size = recv(sock, buffer, BUF_SIZE, 0);
        if (packet_size < 0) {
            perror("Ошибка при получении пакета");
            return 1;
        }

        iph = (struct iphdr *)(buffer);
        udph = (struct udphdr *)(buffer + iph->ihl * 4); // UDP заголовок, исправили на iph->ihl

        // Получение данных
        char *data = buffer + iph->ihl * 4 + sizeof(struct udphdr); // Полезная нагрузка

        // Печать информации
        printf("Источник: %s:%d\n", inet_ntoa(source.sin_addr), ntohs(udph->source));
        printf("Получатель: %s:%d\n", inet_ntoa(dest.sin_addr), ntohs(udph->dest));
        printf("Данные: %s\n", data);
    }

    close(sock);
    return 0;
}
