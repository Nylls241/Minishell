#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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
            continue;
        }

        // Fork a child process to execute other commands
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Code for the child process

            // Tokenize input for execvp
            char *token;
            char *args[256];
            int i = 0;

            token = cmd;
            args[i++] = token;

            while ((token = strtok(NULL, " \t")) != NULL) {
                args[i++] = token;
            }

            args[i] = NULL; // Null-terminate the args array

            // Execute the command
            if (execvp(args[0], args) == -1) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        } else {
            // Parent process waits for child
            int status;
            if (waitpid(pid, &status, 0) == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }
        }
    }

    return 0;
}

