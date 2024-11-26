#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

volatile sig_atomic_t file_access = 1; // Флаг доступа к файлу
volatile sig_atomic_t running = 1; // Флаг работы дочернего процесса

// Обработчик сигнала SIGUSR1
void handle_sigusr1(int sig) {
    file_access = 0; // Блокировка доступа к файлу
}

// Обработчик сигнала SIGUSR2
void handle_sigusr2(int sig) {
    file_access = 1; // Разрешение доступа к файлу
}

// Обработчик сигнала SIGTERM
void handle_sigterm(int sig) {
    running = 0; // Установка флага завершения работы
}

void generate_numbers(int count) {
    FILE *file = fopen("04.txt", "a"); // Открытие файла для добавления
    if (file == NULL) {
        perror("Ошибка открытия файла");
        exit(1);
    }

    srand(time(NULL)); // Инициализация генератора случайных чисел
    for (int i = 0; i < count; i++) {
        int number = rand() % 1000 + 1; // Генерация случайного числа от 1 до 100
        
        // Ожидание разрешения на доступ к файлу
        while (!file_access) {
            pause(); // Приостановка процесса до получения сигнала
        }

        // Блокировка доступа к файлу
        raise(SIGUSR1);

        fprintf(file, "%d\n", number); // Запись числа в файл

        // Разрешение доступа к файлу
        raise(SIGUSR2);
        sleep(1); // Задержка для имитации времени генерации
    }

    fclose(file); // Закрытие файла
}

void read_numbers() {
    while (running) { // Чтение продолжается, пока флаг running = 1
        sleep(2); // Задержка перед чтением, чтобы дать время на запись

        // Ожидание разрешения на доступ к файлу
        while (!file_access) {
            pause(); // Приостановка процесса до получения сигнала
        }

        // Блокировка доступа к файлу
        raise(SIGUSR1);

        FILE *file = fopen("04.txt", "r"); // Открытие файла для чтения
        if (file == NULL) {
            perror("Ошибка открытия файла");
            exit(1);
        }

        printf("Содержимое файла:\n");
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            printf("%s", line); // Вывод содержимого файла
        }
        fclose(file); // Закрытие файла

        // Разрешение доступа к файлу
        raise(SIGUSR2);
        
        sleep(2); // Задержка перед следующим чтением
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <количество_чисел>\n", argv[0]);
        return 1;
    }

    int count = atoi(argv[1]);
    if (count <= 0) {
        fprintf(stderr, "Ошибка: количество чисел должно быть положительным.\n");
        return 1;
    }

    signal(SIGUSR1, handle_sigusr1); // Установка обработчика сигнала SIGUSR1
    signal(SIGUSR2, handle_sigusr2); // Установка обработчика сигнала SIGUSR2
    signal(SIGTERM, handle_sigterm); // Установка обработчика сигнала SIGTERM

    pid_t pid = fork(); // Создание дочернего процесса
    if (pid < 0) {
        perror("Ошибка при создании процесса");
        return 1;
    }

    if (pid == 0) { // Дочерний процесс
        read_numbers(); // Чтение чисел из файла
        exit(0);
    } else { // Родительский процесс
        generate_numbers(count); // Генерация чисел и запись в файл
        kill(pid, SIGTERM); // Завершение дочернего процесса
        wait(NULL); // Ожидание завершения дочернего процесса
    }

    return 0;
}

