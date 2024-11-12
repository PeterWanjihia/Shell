#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>

#define PROMPT_FILE "prompt_setting.txt"
#define HISTORY_SIZE 5

char *token;
char history[HISTORY_SIZE][50];
int history_count = 0;
int current_history_index = -1;

void set_prompt(char *command, char *new_prompt) {
    // The function expects "setprompt" as command followed by a new prompt name
    if (strcmp(command, "setprompt") == 0) {
        if (strcmp(token, "time") == 0) {
            // Fetch the current system time and format it as HH:MM:SS>
            time_t t;
            struct tm *tm_info;
            char time_str[9];

            time(&t);
            tm_info = localtime(&t);
            strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
            snprintf(new_prompt, 50, "%s>", time_str);
        } else {
            // Append '>' to the new prompt name
            snprintf(new_prompt, 50, "%s>", token);
        }
        // Save the new prompt setting to a file
        FILE *file = fopen(PROMPT_FILE, "w");
        if (file != NULL) {
            fprintf(file, "%s", new_prompt);
            fclose(file);
        }
    }
}

void load_prompt(char *prompt) {
    FILE *file = fopen(PROMPT_FILE, "r");
    if (file != NULL) {
        fgets(prompt, 50, file);
        fclose(file);
    } else {
        strcpy(prompt, "$>");
    }
}

int case_insensitive_compare(const char *str1, const char *str2) {
    while (*str1 && *str2) {
        if (tolower((unsigned char)*str1) != tolower((unsigned char)*str2)) {
            return 0;
        }
        str1++;
        str2++;
    }
    return *str1 == *str2;
}

void add_to_history(const char *command) {
    // Add command to history, implementing a circular buffer
    strncpy(history[history_count % HISTORY_SIZE], command, 50);
    history[history_count % HISTORY_SIZE][49] = '\0';  // Ensure null termination
    history_count++;
}

void print_history() {
    int start = (history_count > HISTORY_SIZE) ? history_count - HISTORY_SIZE : 0;
    for (int i = start; i < history_count; i++) {
        printf("%d: %s\n", i - start + 1, history[i % HISTORY_SIZE]);
    }
}

void handle_arrow_keys() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    char c;
    read(STDIN_FILENO, &c, 1);
    if (c == '[') {
        read(STDIN_FILENO, &c, 1);
        if (c == 'A') {  // Up arrow key
            if (history_count > 0) {
                current_history_index = (current_history_index - 1 + HISTORY_SIZE) % HISTORY_SIZE;
                if (current_history_index >= 0 && current_history_index < history_count) {
                    printf("\r%s %s", history[current_history_index], "                                             ");
                }
            }
        } else if (c == 'B') {  // Down arrow key
            if (history_count > 0) {
                current_history_index = (current_history_index + 1) % HISTORY_SIZE;
                if (current_history_index >= 0 && current_history_index < history_count) {
                    printf("\r%s %s", history[current_history_index], "                                             ");
                }
            }
        }
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

int main() {
    char prompt[50];
    load_prompt(prompt);

    while (1) {
        char input[50];
        
        // Print the current prompt
        printf("%s ", prompt);

        // Get user input
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;  // Remove the newline character from input

        // Add command to history
        if (strlen(input) > 0) {
            add_to_history(input);
        }

        // Tokenize the input
        token = strtok(input, " ");

        // If token is "setprompt", call set_prompt
        if (token != NULL && strcmp(token, "setprompt") == 0) {
            token = strtok(NULL, " ");  // Get the new prompt value
            if (token != NULL) {
                set_prompt("setprompt", prompt);
            } else {
                printf("Error: No prompt specified\n");
            }
        } else if (token != NULL && case_insensitive_compare(token, "exit")) {
            printf("Goodbye!\n");
            break;
        } else if (token != NULL && case_insensitive_compare(token, "history")) {
            print_history();
        } else if (token != NULL) {
            // Handle other commands by printing each argument on a separate line
            do {
                printf("%s\n", token);
                token = strtok(NULL, " ");
            } while (token != NULL);
        }
    }

    return 0;
}
