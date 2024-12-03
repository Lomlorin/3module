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

int nclients = 0;

void printusers() {  
    printf("%d пользователь(ей) в сети\n", nclients); 
}

void dostuff(int sock) { 
    char buff[1024];
    int a, b;
    char operation;
    char response[1024];

    // отправляем клиенту инструкции
    const char *msg = "Введите операцию в формате: <число> <операция> <число>\n"
                      "Поддерживаемые операции: + - * /\n"
                      "Для выхода введите 'выход'.\n";
    write(sock, msg, strlen(msg));

    while (1) {
        // читаем данные от клиента
        int bytes_recv = read(sock, buff, sizeof(buff) - 1);
        if (bytes_recv <= 0) break;

        buff[bytes_recv] = '\0';

        // обработка команды выхода
        if (strncmp(buff, "выход", 5) == 0) {
            printf("Клиент отключился.\n");
            break;
        }

        // парсинг команды
        if (sscanf(buff, "%d %c %d", &a, &operation, &b) != 3) {
            const char *error_msg = "Ошибка ввода. Формат: <число> <операция> <число>\n";
            write(sock, error_msg, strlen(error_msg));
            continue;
        }

        // вычисление результата
        double result;
        int valid = 1;

        switch (operation) {
            case '+': result = a + b; break;
            case '-': result = a - b; break;
            case '*': result = a * b; break;
            case '/': 
                if (b != 0) result = (double)a / b; 
                else {
                    valid = 0;
                    snprintf(response, sizeof(response), "Ошибка: деление на ноль\n");
                }
                break;
            default: 
                valid = 0;
                snprintf(response, sizeof(response), "Ошибка: неподдерживаемая операция '%c'\n", operation);
        }

        if (valid) {
            snprintf(response, sizeof(response), "Результат: %.2f\n", result);
        }

        // отправляем результат клиенту
        write(sock, response, strlen(response));
    }

    nclients--;
    printusers();
    close(sock);
}

int main(int argc, char *argv[]) { 
    int sockfd, newsockfd, portno; 
    socklen_t clilen; 
    struct sockaddr_in serv_addr, cli_addr; 

    if (argc < 2) {
        fprintf(stderr, "ОШИБКА: не указан порт\n");
        exit(1); 
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd < 0) error("ОШИБКА открытия сокета"); 

    bzero((char*)&serv_addr, sizeof(serv_addr)); 
    portno = atoi(argv[1]); 
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_addr.s_addr = INADDR_ANY; 
    serv_addr.sin_port = htons(portno); 

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ОШИБКА при связывании"); 

    listen(sockfd, 5); 
    clilen = sizeof(cli_addr);

    while (1) { 
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); 
        if (newsockfd < 0) error("ОШИБКА при принятии подключения"); 

        nclients++;
        printusers();

        if (!fork()) { 
            close(sockfd); 
            dostuff(newsockfd); 
            exit(0); 
        } else close(newsockfd); 
    }
    close(sockfd); 
    return 0; 
}
