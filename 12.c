/*Алгоритм:

Родительский процесс генерирует наборы случайных чисел.
Родитель помещает эти наборы в разделяемую память.
Дочерний процесс находит максимальное и минимальное число в наборе, и помещает эти значения обратно в разделяемую память.
Родительский процесс выводит максимальное и минимальное значения на экран.
Процесс повторяется до получения сигнала SIGINT.
После завершения работы программа выводит количество обработанных наборов данных.*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h> // Для использования wait()


#define SHM_KEY 1234  // Ключ для сегмента разделяемой памяти
#define SEM_KEY 5678  // Ключ для семафора

// Структура для разделяемой памяти
typedef struct {
    int numbers[100];  // Массив случайных чисел
    int min;           // Минимальное число
    int max;           // Максимальное число
    int count;         // Количество чисел в наборе
} SharedData;

int sem_id;  // ID семафора
int shm_id;  // ID разделяемой памяти
int processed_sets = 0;  // Счётчик обработанных наборов данных

// Обработчик сигнала SIGINT
void handle_sigint(int sig) {
    printf("\nРодитель: Обработано %d наборов данных. Завершаю работу.\n", processed_sets);
    shmctl(shm_id, IPC_RMID, NULL);  // Удаление сегмента разделяемой памяти
    semctl(sem_id, 0, IPC_RMID);     // Удаление семафора
    exit(0);  // Завершаем программу
}

void sem_wait_func() {
    struct sembuf sem_op = {0, -1, 0};
    semop(sem_id, &sem_op, 1);
}

void sem_post_func() {
    struct sembuf sem_op = {0, 1, 0};
    semop(sem_id, &sem_op, 1);
}

void generate_data() {
    SharedData *shm_ptr = (SharedData *)shmat(shm_id, NULL, 0);  // Привязка разделяемой памяти
    srand(time(NULL));

    shm_ptr->count = rand() % 10 + 1;  // Генерация случайного количества чисел (1-10)
    for (int i = 0; i < shm_ptr->count; i++) {
        shm_ptr->numbers[i] = rand() % 100 + 1;  // Заполнение массива случайными числами
    }

    printf("Родитель: Сгенерирован набор из %d чисел: ", shm_ptr->count);
    for (int i = 0; i < shm_ptr->count; i++) {
        printf("%d ", shm_ptr->numbers[i]);
    }
    printf("\n");

    // Сигнализируем дочернему процессу о наличии данных
    sem_post_func();
}

void print_results() {
    SharedData *shm_ptr = (SharedData *)shmat(shm_id, NULL, 0);  // Привязка разделяемой памяти
    printf("Родитель: Минимальное число: %d\n", shm_ptr->min);
    printf("Родитель: Максимальное число: %d\n", shm_ptr->max);
}

void process_data() {
    SharedData *shm_ptr = (SharedData *)shmat(shm_id, NULL, 0);  // Привязка разделяемой памяти

    // Ожидаем, пока родительский процесс заполнит данные
    sem_wait_func();  // Ожидаем разрешения от родительского процесса

    // Вычисление минимального и максимального чисел
    int min = shm_ptr->numbers[0];
    int max = shm_ptr->numbers[0];

    for (int i = 1; i < shm_ptr->count; i++) {
        if (shm_ptr->numbers[i] < min) min = shm_ptr->numbers[i];
        if (shm_ptr->numbers[i] > max) max = shm_ptr->numbers[i];
    }

    // Запись результатов в разделяемую память
    shm_ptr->min = min;
    shm_ptr->max = max;

    printf("Дочерний: Обработаны данные. Минимальное число: %d, Максимальное число: %d\n", min, max);

    // Отправляем сигнал родительскому процессу о завершении
    sem_post_func();  // Разрешаем родительскому процессу продолжить
}

int main() {
    signal(SIGINT, handle_sigint);  // Установка обработчика для сигнала SIGINT

    shm_id = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);  // Создание сегмента разделяемой памяти
    if (shm_id < 0) {
        perror("Ошибка создания разделяемой памяти");
        exit(1);
    }

    sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666);  // Создание семафора
    if (sem_id < 0) {
        perror("Ошибка создания семафора");
        exit(1);
    }

    semctl(sem_id, 0, SETVAL, 0);  // Инициализация семафора, 0 означает блокировку

    pid_t pid = fork();
    if (pid < 0) {
        perror("Ошибка при создании процесса");
        exit(1);
    }

    if (pid == 0) {  // Дочерний процесс
        while (1) {
            process_data();  // Обработка данных
        }
    } else {  // Родительский процесс
        while (1) {
            generate_data();  // Генерация данных
            sem_wait_func();  // Ожидаем завершения обработки данных дочерним процессом
            print_results();   // Выводим результаты
            processed_sets++;  // Увеличиваем счётчик обработанных наборов
            sleep(1);          // Пауза между наборами
        }
    }

    return 0;
}
