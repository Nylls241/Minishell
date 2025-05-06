#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Main function of the shell
 * @return 0 if everything goes well
 */
int main() {
    char input[1024]; ///< Buffer to store user input
    char cwd[1024];   ///< Buffer to store current working directory

    while (1) {
        // Get and display the current working directory
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s%% ", cwd);
        } else {
            perror("getcwd");
            exit(EXIT_FAILURE);
        }

        // Read user input
        if (fgets(input, sizeof(input), stdin) == NULL) {
            // Exit if CTRL+D is detected (EOF)
            printf("\nGoodbye!\n");
            break;
        }

        // Remove newline character
        input[strcspn(input, "\n")] = '\0';

        // Skip empty input
        if (strlen(input) == 0) continue;

        // Tokenize input to extract command
        char *cmd = strtok(input, " \t");

        if (cmd == NULL) continue;

        // Handle "exit"
        if (strcmp(cmd, "exit") == 0) {
            printf("Goodbye!\n");
            break;
        }

        // Handle "cd"
        if (strcmp(cmd, "cd") == 0) {
            char *arg = strtok(NULL, " \t");
            if (arg != NULL) {
                if (chdir(arg) != 0) {
                    perror("chdir");
                }
            } else {
                fprintf(stderr, "cd: missing argument\n");
            }
            continue; // Skip token printing
        }

        // Tokenize and display all arguments
        char *rest = cmd; // start from cmd already
        while (rest != NULL) {
            printf("%s\n", rest);
            rest = strtok(NULL, " \t");
        }
    }

    return 0;
}

