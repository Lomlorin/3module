#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h>

#define SEM_READ "/sem_readers" // Семафор для счётчика читателей
#define SEM_WRITE "/sem_writer" // Двоичный семафор для записи

volatile sig_atomic_t running = 1; // Флаг работы дочерних процессов

// Обработчик сигнала SIGTERM
void handle_sigterm(int sig) {
    running = 0;
}

void generate_numbers(int count, sem_t *sem_read, sem_t *sem_write) {
    FILE *file = fopen("11.txt", "a"); // Открытие файла для добавления
    if (file == NULL) {
        perror("Ошибка открытия файла");
        exit(1);
    }

    srand(time(NULL)); // Инициализация генератора случайных чисел
    for (int i = 0; i < count; i++) {
        int number = rand() % 1000 + 1; // Генерация случайного числа от 1 до 1000

        // Блокировка записи (ожидание отсутствия читателей)
        if (sem_wait(sem_write) == -1) {
            perror("Ошибка ожидания семафора записи");
            fclose(file);
            exit(1);
        }

        fprintf(file, "%d\n", number); // Запись числа в файл
        printf("Записано число: %d\n", number);

        // Освобождение семафора записи
        if (sem_post(sem_write) == -1) {
            perror("Ошибка освобождения семафора записи");
            fclose(file);
            exit(1);
        }

        sleep(1); // Задержка для имитации времени генерации
    }

    fclose(file); // Закрытие файла
}

void read_numbers(int id, sem_t *sem_read, sem_t *sem_write) {
    while (running) {
        sleep(2); // Задержка перед чтением

        // Увеличение счётчика читателей
        if (sem_wait(sem_read) == -1) {
            perror("Ошибка ожидания семафора счётчика читателей");
            exit(1);
        }

        static int readers = 0; // Локальный счётчик читателей
        readers++;
        if (readers == 1) { // Первый читатель блокирует запись
            if (sem_wait(sem_write) == -1) {
                perror("Ошибка блокировки записи");
                exit(1);
            }
        }
        if (sem_post(sem_read) == -1) {
            perror("Ошибка освобождения семафора счётчика читателей");
            exit(1);
        }

        // Чтение данных из файла
        FILE *file = fopen("11.txt", "r");
        if (file == NULL) {
            perror("Ошибка открытия файла");
        } else {
            printf("Читатель #%d читает файл:\n", id);
            char line[256];
            while (fgets(line, sizeof(line), file)) {
                printf("%s", line);
            }
            fclose(file);
        }

        // Уменьшение счётчика читателей
        if (sem_wait(sem_read) == -1) {
            perror("Ошибка ожидания семафора счётчика читателей");
            exit(1);
        }
        readers--;
        if (readers == 0) { // Последний читатель разрешает запись
            if (sem_post(sem_write) == -1) {
                perror("Ошибка разблокировки записи");
                exit(1);
            }
        }
        if (sem_post(sem_read) == -1) {
            perror("Ошибка освобождения семафора счётчика читателей");
            exit(1);
        }

        sleep(2); // Задержка перед следующим чтением
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Использование: %s <количество_чисел> <количество_читателей>\n", argv[0]);
        return 1;
    }

    int count = atoi(argv[1]);
    int num_readers = atoi(argv[2]);
    if (count <= 0 || num_readers <= 0) {
        fprintf(stderr, "Ошибка: количество чисел и читателей должно быть положительным.\n");
        return 1;
    }

    // Создание/открытие семафоров
    sem_t *sem_read = sem_open(SEM_READ, O_CREAT | O_EXCL, 0666, 1);
    sem_t *sem_write = sem_open(SEM_WRITE, O_CREAT | O_EXCL, 0666, 1);
    if (sem_read == SEM_FAILED || sem_write == SEM_FAILED) {
        perror("Ошибка создания семафоров");
        sem_unlink(SEM_READ);
        sem_unlink(SEM_WRITE);
        return 1;
    }

    signal(SIGTERM, handle_sigterm); // Установка обработчика сигнала SIGTERM

    pid_t pid;
    for (int i = 0; i < num_readers; i++) {
        pid = fork();
        if (pid == 0) { // Дочерний процесс
            read_numbers(i + 1, sem_read, sem_write);
            exit(0);
        } else if (pid < 0) {
            perror("Ошибка при создании процесса");
            sem_close(sem_read);
            sem_close(sem_write);
            sem_unlink(SEM_READ);
            sem_unlink(SEM_WRITE);
            return 1;
        }
    }

    generate_numbers(count, sem_read, sem_write); // Родительский процесс генерирует числа

    // Завершение всех дочерних процессов
    for (int i = 0; i < num_readers; i++) {
        kill(0, SIGTERM);
    }
    while (wait(NULL) > 0); // Ожидание завершения всех процессов

    // Закрытие и удаление семафоров
    sem_close(sem_read);
    sem_close(sem_write);
    sem_unlink(SEM_READ);
    sem_unlink(SEM_WRITE);

    return 0;
}
