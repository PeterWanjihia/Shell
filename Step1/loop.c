#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#define PROMPT_FILE "prompt_setting.txt"
#define HISTORY_FILE "history.txt"
#define HISTORY_SIZE 10

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
            snprintf(new_prompt, 50, "%s$", time_str);
        } else {
            // Append '>' to the new prompt name
            snprintf(new_prompt, 50, "%s$", token);
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

void load_history() {
    FILE *file = fopen(HISTORY_FILE, "r");
    if (file != NULL) {
        char line[50];
        while (fgets(line, sizeof(line), file)) {
            line[strcspn(line, "\n")] = 0;  // Remove newline character
            add_to_history(line);
        }
        fclose(file);
    }
}

void save_history() {
    FILE *file = fopen(HISTORY_FILE, "w");
    if (file != NULL) {
        int start = (history_count > HISTORY_SIZE) ? history_count - HISTORY_SIZE : 0;
        for (int i = start; i < history_count; i++) {
            fprintf(file, "%s\n", history[i % HISTORY_SIZE]);
        }
        fclose(file);
    }
}

void print_history() {
    int start = (history_count > HISTORY_SIZE) ? history_count - HISTORY_SIZE : 0;
    for (int i = start; i < history_count; i++) {
        printf("%d: %s\n", i - start + 1, history[i % HISTORY_SIZE]);
    }
}

void enable_raw_mode(struct termios *orig_termios) {
    struct termios raw = *orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void disable_raw_mode(struct termios *orig_termios) {
    tcsetattr(STDIN_FILENO, TCSANOW, orig_termios);
}

int main() {
    char prompt[50];
    load_prompt(prompt);
    load_history();
    struct termios orig_termios;
    tcgetattr(STDIN_FILENO, &orig_termios);

    while (1) {
        char input[50] = "";
        int input_len = 0;
        current_history_index = history_count;

        // Print the current prompt
        printf("%s ", prompt);
        fflush(stdout);

        // Enable raw mode to read character-by-character
        enable_raw_mode(&orig_termios);

        char c;
        while (1) {
            read(STDIN_FILENO, &c, 1);

            if (c == '\n') { // Enter key
                input[input_len] = '\0';
                printf("\n");
                break;
            } else if (c == 127) { // Backspace key
                if (input_len > 0) {
                    input_len--;
                    printf("\b \b");
                    fflush(stdout);
                }
            } else if (c == 27) { // Arrow keys (escape sequence)
                read(STDIN_FILENO, &c, 1);
                if (c == '[') {
                    read(STDIN_FILENO, &c, 1);
                    if (c == 'A') { // Up arrow key
                        if (history_count > 0) {
                            current_history_index = (current_history_index - 1 + HISTORY_SIZE) % HISTORY_SIZE;
                            if (current_history_index >= 0 && current_history_index < history_count) {
                                // Clear current input
                                printf("\33[2K\r%s %s", prompt, history[current_history_index]);
                                strncpy(input, history[current_history_index], 50);
                                input_len = strlen(input);
                                fflush(stdout);
                            }
                        }
                    } else if (c == 'B') { // Down arrow key
                        if (history_count > 0) {
                            current_history_index = (current_history_index + 1) % HISTORY_SIZE;
                            if (current_history_index >= 0 && current_history_index < history_count) {
                                // Clear current input
                                printf("\33[2K\r%s %s", prompt, history[current_history_index]);
                                strncpy(input, history[current_history_index], 50);
                                input_len = strlen(input);
                                fflush(stdout);
                            } else {
                                // Clear current input
                                printf("\33[2K\r%s ", prompt);
                                input_len = 0;
                                fflush(stdout);
                            }
                        }
                    }
                }
            } else { // Regular character input
                if (input_len < (int)(sizeof(input) - 1)) {
                    input[input_len++] = c;
                    printf("%c", c);
                    fflush(stdout);
                }
            }
        }

        // Disable raw mode
        disable_raw_mode(&orig_termios);

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
            save_history();  // Save history before exiting
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
