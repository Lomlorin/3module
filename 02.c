#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Для execvp()
#include <sys/types.h>
#include <sys/wait.h> // Для wait()

#define MAX_INPUT 256  // Максимальная длина ввода
#define MAX_ARGS 10    // Максимальное количество аргументов

int main() {
    char input[MAX_INPUT];
    char *args[MAX_ARGS + 1]; // Аргументы +1 для NULL
    char *token;
    pid_t pid;

    while (1) {
        // Вывод приглашения
        printf("Enter the name of the program in the form   ./sum arg arg   > ");
        fflush(stdout);

        // Считывание ввода
        if (fgets(input, MAX_INPUT, stdin) == NULL) {
            printf("\n");
            break; // Завершение при EOF (Ctrl+D)
        }

        // Удаление символа новой строки
        input[strcspn(input, "\n")] = '\0';

        // Если ввод пустой, пропускаем
        if (strlen(input) == 0) {
            continue;
        }

        // Разбиение строки на аргументы
        int argc = 0;
        token = strtok(input, " ");
        while (token != NULL && argc < MAX_ARGS) {
            args[argc++] = token;
            token = strtok(NULL, " ");
        }
        args[argc] = NULL; // Последний аргумент — NULL

        // Проверка на команду выхода
        if (strcmp(args[0], "exit") == 0) {
            printf("Exit.\n");
            break;
        }

        // Создание дочернего процесса
        pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            continue;
        } else if (pid == 0) {
            // Дочерний процесс: запуск команды
            if (execvp(args[0], args) == -1) {
                perror("Command execution failed");
                exit(EXIT_FAILURE);
            }
        } else {
            // Родительский процесс: ожидание завершения дочернего
            int status;
            waitpid(pid, &status, 0);
        }
    }

    return 0;
}
