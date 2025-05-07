#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

/**
 * @brief Trim leading and trailing spaces and tabs from a string
 * @param str The string to trim
 * @return The trimmed string
 */
char *trim(char *str) {
    while (*str == ' ' || *str == '\t') str++;  // Trim leading
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t')) *end-- = '\0';  // Trim trailing
    return str;
}

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
            printf("\nGoodbye!\n");
            break;
        }

        // Remove newline character
        input[strcspn(input, "\n")] = '\0';

        // Trim input
        char *trimmed_input = trim(input);

        // Skip empty input
        if (strlen(trimmed_input) == 0) continue;

        // Handle "exit"
        if (strcmp(trimmed_input, "exit") == 0) {
            printf("Goodbye!\n");
            break;
        }

        // Handle "cd"
        if (strncmp(trimmed_input, "cd", 2) == 0) {
            char *token = strtok(trimmed_input, " ");
            token = strtok(NULL, " ");
            if (token != NULL) {
                if (chdir(token) != 0) {
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
            // Make a copy of the input for processing
            char command_copy[1024];
            strcpy(command_copy, trimmed_input);

            // Check for redirections
            char *output_redirect = strchr(command_copy, '>');
            char *input_redirect = strchr(command_copy, '<');
            char *error_redirect = strstr(command_copy, "2>");

            // Process output redirection
            if (output_redirect != NULL) {
                // Handle case where 2> comes before >
                if (error_redirect != NULL && error_redirect < output_redirect) {
                    *error_redirect = '\0';  // Cut string at 2>
                    char *error_file = trim(error_redirect + 2);
                    
                    // Find end of error file name
                    char *end = strchr(error_file, '>');
                    if (end != NULL) *end = '\0';
                    error_file = trim(error_file);
                    
                    int fd = open(error_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd == -1) {
                        perror("open");
                        exit(EXIT_FAILURE);
                    }
                    dup2(fd, STDERR_FILENO);
                    close(fd);
                    
                    // Adjust output_redirect position since we modified the string
                    output_redirect = strchr(command_copy, '>');
                }
                
                *output_redirect = '\0';  // Cut string at >
                char *output_file = trim(output_redirect + 1);
                
                // Find end of output file name if other redirections follow
                char *end = strchr(output_file, '<');
                if (end == NULL) end = strstr(output_file, "2>");
                if (end != NULL) *end = '\0';
                output_file = trim(output_file);
                
                int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            // Process input redirection
            if (input_redirect != NULL) {
                *input_redirect = '\0';  // Cut string at <
                char *input_file = trim(input_redirect + 1);
                
                // Find end of input file name if other redirections follow
                char *end = strchr(input_file, '>');
                if (end != NULL) *end = '\0';
                input_file = trim(input_file);
                
                int fd = open(input_file, O_RDONLY);
                if (fd == -1) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            // Process error redirection if not already handled
            if (error_redirect != NULL && (output_redirect == NULL || error_redirect > output_redirect)) {
                *error_redirect = '\0';  // Cut string at 2>
                char *error_file = trim(error_redirect + 2);
                
                int fd = open(error_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                dup2(fd, STDERR_FILENO);
                close(fd);
            }

            // Tokenize command part (without redirections) for execvp
            char *token;
            char *args[256];
            int i = 0;

            token = strtok(command_copy, " \t");
            while (token != NULL && i < 255) {
                args[i++] = token;
                token = strtok(NULL, " \t");
            }
            args[i] = NULL;

            if (i > 0 && execvp(args[0], args) == -1) {
                perror("execvp");
                exit(EXIT_FAILURE);
            } else if (i == 0) {
                fprintf(stderr, "No command specified\n");
                exit(EXIT_FAILURE);
            }
        } else {
            // Parent waits
            int status;
            if (waitpid(pid, &status, 0) == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }
        }
    }

    return 0;
}