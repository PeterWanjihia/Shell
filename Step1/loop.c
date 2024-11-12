#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define PROMPT_FILE "prompt_setting.txt"

char *token;

void set_prompt(char *command, char *new_prompt){
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
        }
    }

    return 0;
}
