#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <wait.h>

int reader_count = 0; // Счетчик активных читателей
int reader_count_sem; // Семафор для защиты reader_count
int write_sem;        // Семафор для контроля записи

// Функции для работы с семафорами
void lock_semaphore(int sem_id) {
    struct sembuf lock = {0, -1, 0}; // Уменьшить значение семафора на 1
    if (semop(sem_id, &lock, 1) == -1) {
        perror("Ошибка блокировки семафора");
        exit(1);
    }
}

void unlock_semaphore(int sem_id) {
    struct sembuf unlock = {0, 1, 0}; // Увеличить значение семафора на 1
    if (semop(sem_id, &unlock, 1) == -1) {
        perror("Ошибка разблокировки семафора");
        exit(1);
    }
}

// Чтение содержимого файла
void read_numbers() {
    while (1) {
        // Увеличиваем счетчик читателей
        lock_semaphore(reader_count_sem);
        reader_count++;
        if (reader_count == 1) {
            // Если это первый читатель, блокируем запись
            lock_semaphore(write_sem);
        }
        unlock_semaphore(reader_count_sem);

        // Чтение файла
        FILE *file = fopen("08.txt", "r");
        if (file == NULL) {
            perror("Ошибка открытия файла");
            exit(1);
        }

        printf("Читатель %d читает содержимое файла:\n", getpid());
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            printf("%s", line);
        }
        fclose(file);
        sleep(2); // Задержка для имитации чтения

        // Уменьшаем счетчик читателей
        lock_semaphore(reader_count_sem);
        reader_count--;
        if (reader_count == 0) {
            // Если это последний читатель, разблокируем запись
            unlock_semaphore(write_sem);
        }
        unlock_semaphore(reader_count_sem);
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
        int number = rand() % 1000 + 1;

        // Блокируем запись
        lock_semaphore(write_sem);

        fprintf(file, "%d\n", number);
        printf("Писатель %d записал число: %d\n", getpid(), number);

        // Разблокируем запись
        unlock_semaphore(write_sem);

        sleep(1); // Задержка для имитации работы
    }

    fclose(file);
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

    // Создание ключей для семафоров
    key_t key1 = ftok("08.txt", 'R');
    key_t key2 = ftok("08.txt", 'W');
    if (key1 == -1 || key2 == -1) {
        perror("Ошибка создания ключа");
        return 1;
    }

    // Создание семафоров
    reader_count_sem = semget(key1, 1, IPC_CREAT | 0666);
    write_sem = semget(key2, 1, IPC_CREAT | 0666);
    if (reader_count_sem == -1 || write_sem == -1) {
        perror("Ошибка создания семафора");
        return 1;
    }

    // Инициализация семафоров
    if (semctl(reader_count_sem, 0, SETVAL, 1) == -1 || semctl(write_sem, 0, SETVAL, 1) == -1) {
        perror("Ошибка инициализации семафора");
        return 1;
    }

    pid_t pid1 = fork();
    if (pid1 == 0) { // Первый читатель
        read_numbers();
        exit(0);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) { // Второй читатель
        read_numbers();
        exit(0);
    }

    // Родительский процесс — писатель
    generate_numbers(count);

    // Завершаем дочерние процессы
    kill(pid1, SIGTERM);
    kill(pid2, SIGTERM);
    wait(NULL);
    wait(NULL);

    // Удаляем семафоры
    if (semctl(reader_count_sem, 0, IPC_RMID) == -1 || semctl(write_sem, 0, IPC_RMID) == -1) {
        perror("Ошибка удаления семафоров");
        return 1;
    }

    return 0;
}
