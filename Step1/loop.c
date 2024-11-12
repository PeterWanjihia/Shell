#include <stdio.h>
#include <string.h>
#include <time.h>

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
    }
}

int main() {
    char prompt[50] = "$";

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
        }
    }

    return 0;
}