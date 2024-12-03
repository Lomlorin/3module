#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h>

#define SEM_NAME "/file_semaphore"

volatile sig_atomic_t running = 1; // Флаг работы дочернего процесса

// Обработчик сигнала SIGTERM
void handle_sigterm(int sig) {
    running = 0; // Установка флага завершения работы
}

void generate_numbers(int count, sem_t *sem) {
    FILE *file = fopen("05.txt", "a"); // Открытие файла для добавления
    if (file == NULL) {
        perror("Ошибка открытия файла");
        exit(1);
    }

    srand(time(NULL)); // Инициализация генератора случайных чисел
    for (int i = 0; i < count; i++) {
        int number = rand() % 1000 + 1; // Генерация случайного числа от 1 до 1000

        // Блокировка доступа к файлу с помощью семафора
        if (sem_wait(sem) == -1) {
            perror("Ошибка ожидания семафора");
            fclose(file);
            exit(1);
        }

        fprintf(file, "%d\n", number); // Запись числа в файл
        printf("Записано число: %d\n", number);

        // Освобождение семафора
        if (sem_post(sem) == -1) {
            perror("Ошибка освобождения семафора");
            fclose(file);
            exit(1);
        }

        sleep(1); // Задержка для имитации времени генерации
    }

    fclose(file); // Закрытие файла
}

void read_numbers(sem_t *sem) {
    while (running) {
        sleep(2); // Задержка перед чтением, чтобы дать время на запись

        // Блокировка доступа к файлу с помощью семафора
        if (sem_wait(sem) == -1) {
            perror("Ошибка ожидания семафора");
            exit(1);
        }

        FILE *file = fopen("05.txt", "r"); // Открытие файла для чтения
        if (file == NULL) {
            perror("Ошибка открытия файла");
            sem_post(sem); // Освобождение семафора при ошибке
            exit(1);
        }

        printf("Содержимое файла:\n");
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            printf("%s", line); // Вывод содержимого файла
        }
        fclose(file); // Закрытие файла

        // Освобождение семафора
        if (sem_post(sem) == -1) {
            perror("Ошибка освобождения семафора");
            exit(1);
        }

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

    // Создание/открытие семафора
    sem_t *sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("Ошибка создания семафора");
        return 1;
    }

    signal(SIGTERM, handle_sigterm); // Установка обработчика сигнала SIGTERM

    pid_t pid = fork(); // Создание дочернего процесса
    if (pid < 0) {
        perror("Ошибка при создании процесса");
        sem_close(sem);
        sem_unlink(SEM_NAME);
        return 1;
    }

    if (pid == 0) { // Дочерний процесс
        read_numbers(sem); // Чтение чисел из файла
        exit(0);
    } else { // Родительский процесс
        generate_numbers(count, sem); // Генерация чисел и запись в файл
        kill(pid, SIGTERM); // Завершение дочернего процесса
        wait(NULL); // Ожидание завершения дочернего процесса
    }

    // Закрытие и удаление семафора
    if (sem_close(sem) == -1) {
        perror("Ошибка закрытия семафора");
    }
    if (sem_unlink(SEM_NAME) == -1) {
        perror("Ошибка удаления семафора");
    }

    return 0;
}
