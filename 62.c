#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define MSG_SIZE 256
#define MSG_TYPE 1
#define TERMINATE_TYPE 2

struct message {
    long msg_type;
    char msg_text[MSG_SIZE];
};

int main() {
    key_t key = ftok("chatfile", 65); // Генерация ключа
    int msgid = msgget(key, 0666 | IPC_CREAT); // Получение очереди сообщений

    struct message msg;

    while (1) {
        // Получение сообщения от отправителя
        msgrcv(msgid, &msg, sizeof(msg.msg_text), MSG_TYPE, 0);
        printf("Отправитель: %s", msg.msg_text);

        // Проверка на завершение
        if (msg.msg_type == TERMINATE_TYPE) {
            break;
        }

        // Отправка ответа
        printf("Получатель: ");
        fgets(msg.msg_text, MSG_SIZE, stdin);
        msg.msg_type = MSG_TYPE;
        msgsnd(msgid, &msg, sizeof(msg.msg_text), 0); // Отправка ответа
    }

    msgctl(msgid, IPC_RMID, NULL); // Удаление очереди сообщений
    return 0;
}
