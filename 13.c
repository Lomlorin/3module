#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/wait.h>

#define SHM_NAME "/shared_data"  // Имя для разделяемой памяти
#define SEM_NAME "/semaphore"    // Имя для семафора

// Структура для разделяемой памяти
typedef struct {
    int numbers[100];  // Массив случайных чисел
    int min;           // Минимальное число
    int max;           // Максимальное число
    int count;         // Количество чисел в наборе
} SharedData;

sem_t *sem;  // Семафор
SharedData *shm_ptr;  // Указатель на разделяемую память
int processed_sets = 0;  // Счётчик обработанных наборов данных

// Обработчик сигнала SIGINT
void handle_sigint(int sig) {
    printf("\nРодитель: Обработано %d наборов данных. Завершаю работу.\n", processed_sets);
    shm_unlink(SHM_NAME);  // Удаление разделяемой памяти
    sem_close(sem);  // Закрытие семафора
    sem_unlink(SEM_NAME);  // Удаление семафора
    exit(0);  // Завершаем программу
}

void sem_wait_func() {
    sem_wait(sem);  // Ожидание семафора
}

void sem_post_func() {
    sem_post(sem);  // Разрешение семафора
}

void generate_data() {
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
    printf("Родитель: Минимальное число: %d\n", shm_ptr->min);
    printf("Родитель: Максимальное число: %d\n", shm_ptr->max);
}

void process_data() {
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

    // Создание разделяемой памяти POSIX
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Ошибка создания разделяемой памяти");
        exit(1);
    }
    ftruncate(shm_fd, sizeof(SharedData));  // Установка размера разделяемой памяти
    shm_ptr = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("Ошибка отображения разделяемой памяти");
        exit(1);
    }

    // Создание и инициализация семафора POSIX
    sem = sem_open(SEM_NAME, O_CREAT, 0666, 0);  // Инициализация семафора (значение 0)
    if (sem == SEM_FAILED) {
        perror("Ошибка создания семафора");
        exit(1);
    }

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
