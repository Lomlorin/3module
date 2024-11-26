#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <wait.h>

int sem_id; // Идентификатор семафора

// Функции для работы с семафорами
void lock_semaphore() {
    struct sembuf lock = {0, -1, 0}; // Операция: уменьшить семафор на 1
    if (semop(sem_id, &lock, 1) == -1) {
        perror("Ошибка блокировки семафора");
        exit(1);
    }
}

void unlock_semaphore() {
    struct sembuf unlock = {0, 1, 0}; // Операция: увеличить семафор на 1
    if (semop(sem_id, &unlock, 1) == -1) {
        perror("Ошибка разблокировки семафора");
        exit(1);
    }
}

// Генерация случайных чисел и запись в файл
void generate_numbers(int count) {
    FILE *file = fopen("08.txt", "a");
    if (file == NULL) {
        perror("Ошибка открытия файла");
        exit(1);
    }

    srand(time(NULL));
    for (int i = 0; i < count; i++) {
        int number = rand() % 1000 + 1; // Генерация случайного числа

        lock_semaphore(); // Блокировка доступа к файлу

        fprintf(file, "%d\n", number); // Запись числа в файл

        unlock_semaphore(); // Разблокировка доступа к файлу

        sleep(1); // Задержка для имитации времени работы
    }

    fclose(file);
}

// Чтение содержимого файла
void read_numbers() {
    while (1) {
        sleep(2); // Задержка перед чтением

        lock_semaphore(); // Блокировка доступа к файлу

        FILE *file = fopen("08.txt", "r");
        if (file == NULL) {
            perror("Ошибка открытия файла");
            exit(1);
        }

        printf("Содержимое файла:\n");
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            printf("%s", line);
        }
        fclose(file);

        unlock_semaphore(); // Разблокировка доступа к файлу
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

    // Создание ключа для семафора
    key_t key = ftok("08.txt", 'A');
    if (key == -1) {
        perror("Ошибка создания ключа");
        return 1;
    }

    // Создание семафора
    sem_id = semget(key, 1, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("Ошибка создания семафора");
        return 1;
    }

    // Инициализация семафора (устанавливаем значение 1)
    if (semctl(sem_id, 0, SETVAL, 1) == -1) {
        perror("Ошибка инициализации семафора");
        return 1;
    }

    pid_t pid = fork();
    if (pid == 0) { // Дочерний процесс
        read_numbers(); // Чтение чисел из файла
    } else if (pid > 0) { // Родительский процесс
        generate_numbers(count); // Генерация чисел и запись в файл
        kill(pid, SIGTERM); // Завершение дочернего процесса
        wait(NULL); // Ожидание завершения дочернего процесса

        // Удаление семафора
        if (semctl(sem_id, 0, IPC_RMID) == -1) {
            perror("Ошибка удаления семафора");
            return 1;
        }
    } else {
        perror("Ошибка при создании процесса");
        return 1;
    }

    return 0;
}
