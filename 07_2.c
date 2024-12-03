#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>

#define QUEUE_NAME "/chat_queue"
#define MAX_SIZE 1024
#define TERMINATE_PRIORITY 99

void error_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main() {
    mqd_t mq;
    char buffer[MAX_SIZE];
    unsigned int priority;

    // Открытие очереди
    mq = mq_open(QUEUE_NAME, O_RDWR);
    if (mq == (mqd_t)-1) {
        error_exit("Ошибка открытия очереди");
    }

    printf("Ожидание сообщений...\n");
    while (1) {
        // Получение сообщения
        ssize_t bytes_read = mq_receive(mq, buffer, MAX_SIZE, &priority);
        if (bytes_read == -1) {
            error_exit("Ошибка получения сообщения");
        }
        buffer[bytes_read] = '\0';

        // Проверка на завершение
        if (priority == TERMINATE_PRIORITY) {
            printf("Чат завершён.\n");
            break;
        }

        printf("Собеседник: %s\n", buffer);

        // Ответ на сообщение
        printf("Вы: ");
        fgets(buffer, MAX_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "exit") == 0) {
            if (mq_send(mq, buffer, strlen(buffer) + 1, TERMINATE_PRIORITY) == -1) {
                error_exit("Ошибка отправки сообщения");
            }
            break;
        }

        if (mq_send(mq, buffer, strlen(buffer) + 1, 1) == -1) {
            error_exit("Ошибка отправки сообщения");
        }
    }

    // Закрытие и удаление очереди
    if (mq_close(mq) == -1) {
        error_exit("Ошибка закрытия очереди");
    }
    if (mq_unlink(QUEUE_NAME) == -1) {
        error_exit("Ошибка удаления очереди");
    }

    return 0;
}
