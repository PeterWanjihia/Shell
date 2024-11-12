#include <stdio.h>
#include <string.h>

#define MAX_INPUT_SIZE 1024

void execute_builtin(char *command) {
    if (strcmp(command, "hello") == 0) {
        printf("Hello, World!\n");
    } else {
        printf("Unknown command: %s\n", command);
    }
}

void set_prompt(char *command){
    if(strcmp(command,"setprompt")== 0){
        
    }
}

int main() {
    char input[MAX_INPUT_SIZE];

    while (1) {
        // Print shell prompt
        printf("CustomShell> ");
        
        // Get user input
        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
            perror("fgets error");
            continue;
        }

        // Remove trailing newline character
        input[strcspn(input, "\n")] = '\0';

        // Skip empty input
        if (strlen(input) == 0) {
            continue;
        }

        // Check if it's the built-in command "hello"
        if (strcmp(input, "hello") == 0) {
            // Execute the built-in command
            execute_builtin(input);
        } else {
            // Inform user if the command is not recognized
            printf("Unknown command: %s\n", input);
        }
    }

    return 0;
}
