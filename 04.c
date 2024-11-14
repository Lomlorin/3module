#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

#define PIPE_NAME "data_pipe" // Имя канала

void generate_random_numbers(int count, int write_fd) {
    // Генерация случайных чисел и запись их в pipe
    srand(time(NULL));
    for (int i = 0; i < count; i++) {
        int random_number = rand() % 1000; // Генерация случайного числа
        write(write_fd, &random_number, sizeof(random_number)); // Отправка числа родителю
    }
}

void parent_process(int read_fd, int count) {
    // Родительский процесс получает данные из pipe и выводит их на экран и в файл
    FILE *file = fopen("04.txt", "w");
    if (file == NULL) {
        perror("Failed to open file");
        exit(1);
    }

    int number;
    for (int i = 0; i < count; i++) {
        read(read_fd, &number, sizeof(number)); // Чтение числа из pipe
        printf("Received number: %d\n", number); // Вывод числа на экран
        fprintf(file, "%d\n", number); // Запись числа в файл
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <count_of_numbers>\n", argv[0]);
        exit(1);
    }

    int count = atoi(argv[1]); // Чтение количества чисел

    // Проверяем на валидность входных данных
    if (count <= 0) {
        fprintf(stderr, "Please enter a valid number greater than 0.\n");
        exit(1);
    }

    int pipe_fd[2]; // Массив для хранения дескрипторов pipe

    if (pipe(pipe_fd) == -1) {
        perror("Failed to create pipe");
        exit(1);
    }

    pid_t pid = fork(); // Создаем дочерний процесс

    if (pid == -1) {
        perror("Failed to create child process");
        exit(1);
    }

    if (pid == 0) {
        // Дочерний процесс
        close(pipe_fd[0]); // Закрываем конец для чтения
        generate_random_numbers(count, pipe_fd[1]); // Генерируем случайные числа и отправляем их в pipe
        close(pipe_fd[1]); // Закрываем конец для записи
        exit(0);
    } else {
        // Родительский процесс
        close(pipe_fd[1]); // Закрываем конец для записи
        parent_process(pipe_fd[0], count); // Получаем числа и выводим их
        close(pipe_fd[0]); // Закрываем конец для чтения
    }

    return 0;
}
