#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <unistd.h>    // Для fork()

// Функция для расчета площадей квадратов
void calculate_areas(int *sides, int start, int end, const char *process_name) {
    for (int i = start; i < end; i++) {
        int area = sides[i] * sides[i];
        printf("Side: %d, Area: %d (calculated by %s)\n", sides[i], area, process_name);
    }
}

int main() {
    int num_sides;

    // Запрос количества квадратов
    printf("Enter the number of squares: ");
    if (scanf("%d", &num_sides) != 1 || num_sides <= 0) {
        fprintf(stderr, "Error: Please enter a valid positive integer.\n");
        return 1;
    }

    // Выделение памяти для длин сторон
    int *sides = (int *)malloc(num_sides * sizeof(int));
    if (!sides) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return 1;
    }

    // Ввод длин сторон
    printf("Enter the lengths of the sides:\n");
    for (int i = 0; i < num_sides; i++) {
        printf("Side %d: ", i + 1);
        if (scanf("%d", &sides[i]) != 1 || sides[i] <= 0) {
            fprintf(stderr, "Error: Please enter a valid positive integer for side %d.\n", i + 1);
            free(sides);
            return 1;
        }
    }

    // Создание дочернего процесса
    pid_t pid = fork();

    if (pid < 0) {
        // Ошибка fork()
        perror("Fork failed");
        free(sides);
        return 1;
    } else if (pid == 0) {
        // Дочерний процесс
        calculate_areas(sides, num_sides / 2, num_sides, "Child");
    } else {
        // Родительский процесс
        calculate_areas(sides, 0, num_sides / 2, "Parent");
    }

    // Освобождение памяти
    free(sides);

    return 0;
}
