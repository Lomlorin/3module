#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_CONTACTS 100
#define MAX_NAME_LENGTH 50
#define MAX_PHONE_LENGTH 15
#define MAX_EMAIL_LENGTH 50
#define MAX_WORK_LENGTH 50
#define MAX_POSITION_LENGTH 50
#define MAX_SOCIAL_LENGTH 50

typedef struct {
    char first_name[MAX_NAME_LENGTH];
    char surname[MAX_NAME_LENGTH];
    char last_name[MAX_NAME_LENGTH];
    char phone[MAX_PHONE_LENGTH];
    char email[MAX_EMAIL_LENGTH];
    char work[MAX_WORK_LENGTH];
    char position[MAX_POSITION_LENGTH];
    char social[MAX_SOCIAL_LENGTH];
} Contact;

Contact contacts[MAX_CONTACTS];
int contact_count = 0;

void load_contacts(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return; // Если файл не существует, просто вернемся
    }

    while (fscanf(file, "%49s %49s %49s %14s %49s %49s %49s %49s",
                  contacts[contact_count].first_name,
                  contacts[contact_count].surname,
                  contacts[contact_count].last_name,
                  contacts[contact_count].phone,
                  contacts[contact_count].email,
                  contacts[contact_count].work,
                  contacts[contact_count].position,
                  contacts[contact_count].social) == 8) {
        contact_count++;
        if (contact_count >= MAX_CONTACTS) {
            break; // Предотвращаем переполнение массива
        }
    }

    fclose(file);
}

void save_contacts(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Unable to open file for writing");
        return;
    }

    for (int i = 0; i < contact_count; i++) {
        fprintf(file, "%s %s %s %s %s %s %s %s\n",
                contacts[i].first_name,
                contacts[i].surname,
                contacts[i].last_name,
                contacts[i].phone,
                contacts[i].email,
                contacts[i].work,
                contacts[i].position,
                contacts[i].social);
    }

    fclose(file);
}


void add_contact() {
    if (contact_count >= MAX_CONTACTS) {
        write(STDOUT_FILENO, "Contact list is full.\n", 23);
        return;
    }

    write(STDOUT_FILENO, "Enter first name: ", 18);
    read(STDIN_FILENO, contacts[contact_count].first_name, MAX_NAME_LENGTH);
    
    write(STDOUT_FILENO, "Enter surname: ", 15);
    read(STDIN_FILENO, contacts[contact_count].surname, MAX_NAME_LENGTH);
    
    write(STDOUT_FILENO, "Enter last name: ", 17);
    read(STDIN_FILENO, contacts[contact_count].last_name, MAX_NAME_LENGTH);

    write(STDOUT_FILENO, "Enter phone: ", 13);
    read(STDIN_FILENO, contacts[contact_count].phone, MAX_PHONE_LENGTH);

    write(STDOUT_FILENO, "Enter email: ", 13);
    read(STDIN_FILENO, contacts[contact_count].email, MAX_EMAIL_LENGTH);

    write(STDOUT_FILENO, "Enter place of work: ", 21);
    read(STDIN_FILENO, contacts[contact_count].work, MAX_WORK_LENGTH);

    write(STDOUT_FILENO, "Enter position: ", 16);
    read(STDIN_FILENO, contacts[contact_count].position, MAX_POSITION_LENGTH);

    write(STDOUT_FILENO, "Enter social network: ", 22);
    read(STDIN_FILENO, contacts[contact_count].social, MAX_SOCIAL_LENGTH);

    contact_count++;
}

void edit_contact(char *first_name, char *surname) {
    for (int i = 0; i < contact_count; i++) {
        if (strcmp(contacts[i].first_name, first_name) == 0 && strcmp(contacts[i].surname, surname) == 0) {
            write(STDOUT_FILENO, "Editing contact:\n", 18);
            write(STDOUT_FILENO, "Enter new phone: ", 17);
            read(STDIN_FILENO, contacts[i].phone, MAX_PHONE_LENGTH);
            write(STDOUT_FILENO, "Enter new email: ", 17);
            read(STDIN_FILENO, contacts[i].email, MAX_EMAIL_LENGTH);
            write(STDOUT_FILENO, "Enter new place of work: ", 25);
            read(STDIN_FILENO, contacts[i].work, MAX_WORK_LENGTH);
            write(STDOUT_FILENO, "Enter new position: ", 20);
            read(STDIN_FILENO, contacts[i].position, MAX_POSITION_LENGTH);
            write(STDOUT_FILENO, "Enter new social network: ", 25);
            read(STDIN_FILENO, contacts[i].social, MAX_SOCIAL_LENGTH);
            return;
        }
    }
    write(STDOUT_FILENO, "Contact not found.\n", 19);
}

void delete_contact(char *first_name, char *surname) {
    for (int i = 0; i < contact_count; i++) {
        if (strcmp(contacts[i].first_name, first_name) == 0 && strcmp(contacts[i].surname, surname) == 0) {
            for (int j = i; j < contact_count - 1; j++) {
                contacts[j] = contacts[j + 1];
            }
            contact_count--;
            write(STDOUT_FILENO, "Contact deleted.\n", 17);
            return;
        }
    }
    write(STDOUT_FILENO, "Contact not found.\n", 19);
}

void list_contacts() {
    if (contact_count == 0) {
        write(STDOUT_FILENO, "Contact list is empty.\n", 23);
        return;
    }

    for (int i = 0; i < contact_count; i++) {
        dprintf(STDOUT_FILENO, "Contact %d:\n", i + 1);
        dprintf(STDOUT_FILENO, "Name: %s\n", contacts[i].first_name);
        dprintf(STDOUT_FILENO, "Surname: %s\n", contacts[i].surname);
        dprintf(STDOUT_FILENO, "Last name: %s\n", contacts[i].last_name);
        dprintf(STDOUT_FILENO, "Phone: %s\n", contacts[i].phone);
        dprintf(STDOUT_FILENO, "Email: %s\n", contacts[i].email);
        if (strlen(contacts[i].work) > 0) {
            dprintf(STDOUT_FILENO, "Place of work: %s\n", contacts[i].work);
        }
        if (strlen(contacts[i].position) > 0) {
            dprintf(STDOUT_FILENO, "Position: %s\n", contacts[i].position);
        }
        if (strlen(contacts[i].social) > 0) {
            dprintf(STDOUT_FILENO, "Social network: %s\n", contacts[i].social);
        }
        dprintf(STDOUT_FILENO, "\n");
    }
}

int main() {
    const char *filename = "file.txt";
    
    load_contacts(filename);

    int choice;

    while (1) {
        write(STDOUT_FILENO, "Phone book:\n", 12);
        write(STDOUT_FILENO, "1. Add a contact\n", 17);
        write(STDOUT_FILENO, "2. Edit the contact\n", 20);
        write(STDOUT_FILENO, "3. Delete the contact\n", 22);
        write(STDOUT_FILENO, "4. Show all contacts\n", 21);
        write(STDOUT_FILENO, "5. Exit\n", 8);
        write(STDOUT_FILENO, "Select an action: \n", 19);

        scanf("%d", &choice); // Здесь используется буферизованный ввод для выбора действия
        getchar(); // Чистим символ новой строки из буфера

        switch (choice) {
            case 1:
                add_contact();
                break;
            case 2: {
                char first_name2[MAX_NAME_LENGTH], surname2[MAX_NAME_LENGTH];
                write(STDOUT_FILENO, "Enter the first name of the contact to edit:\n", 45);
                read(STDIN_FILENO, first_name2, MAX_NAME_LENGTH);
                write(STDOUT_FILENO,"Enter the surname of the contact to edit:\n", 42);
                read(STDIN_FILENO,surname2 ,MAX_NAME_LENGTH);
                edit_contact(first_name2,surname2 );
                break;
            }
            case 3: {
                char first_name3[MAX_NAME_LENGTH], surname3[MAX_NAME_LENGTH];
                write(STDOUT_FILENO,"Enter the first name of the contact to delete:\n", 47);
                read(STDIN_FILENO ,first_name3 ,MAX_NAME_LENGTH);
                write(STDOUT_FILENO,"Enter the surname of the contact to delete:\n", 44);
                read(STDIN_FILENO ,surname3 ,MAX_NAME_LENGTH);
                delete_contact(first_name3,surname3 );
                break;
            }
            case 4:
                list_contacts();
                break;
            case 5:
                save_contacts(filename); // Сохраняем контакты перед выходом
                exit(0);
            default:
                write(STDOUT_FILENO,"Wrong choice. Try again.\n",25 );
        }
    }

    return 0;
}
