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

int main(int argc, char *argv[]) { 
    int my_sock, portno, n; 
    struct sockaddr_in serv_addr; 
    struct hostent *server; 
    char buff[1024]; 

    if (argc < 3) { 
        fprintf(stderr, "Использование: %s <хост> <порт>\n", argv[0]); 
        exit(0); 
    }

    portno = atoi(argv[2]);
    my_sock = socket(AF_INET, SOCK_STREAM, 0); 
    if (my_sock < 0) error("ОШИБКА открытия сокета"); 

    server = gethostbyname(argv[1]); 
    if (server == NULL) { 
        fprintf(stderr, "ОШИБКА: указанный хост не найден\n"); 
        exit(0); 
    }

    bzero((char*)&serv_addr, sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET; 
    bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length); 
    serv_addr.sin_port = htons(portno);

    if (connect(my_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ОШИБКА подключения");

    while ((n = recv(my_sock, buff, sizeof(buff) - 1, 0)) > 0) { 
        buff[n] = 0; 
        printf("Сервер => Клиент: %s", buff); 

        printf("Клиент => Сервер: ");  
        fgets(buff, sizeof(buff) - 1, stdin);

        if (strncmp(buff, "выход", 5) == 0) { 
            printf("Завершение работы...\n"); 
            close(my_sock); 
            return 0; 
        }

        send(my_sock, buff, strlen(buff), 0); 
    }

    printf("Ошибка получения данных\n"); 
    close(my_sock); 
    return -1; 
}
