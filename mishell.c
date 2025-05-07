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
 * @brief Parse a single command and handle its redirections
 * @param cmd The command string to parse
 * @param args Array to store the parsed arguments
 * @param in_fd Input file descriptor (for redirections)
 * @param out_fd Output file descriptor (for redirections)
 * @param err_fd Error file descriptor (for redirections)
 * @return Number of arguments stored in args
 */
int parse_command(char *cmd, char *args[], int *in_fd, int *out_fd, int *err_fd) {
    char *input_redirect = NULL;
    char *output_redirect = NULL;
    char *error_redirect = NULL;
    int arg_count = 0;
    
    // Find redirections
    char *current = cmd;
    while (*current != '\0') {
        if (*current == '<' && (current == cmd || *(current-1) != '2')) {
            input_redirect = current;
            *current = '\0';
            current++;
        } else if (*current == '>' && (current == cmd || *(current-1) != '2')) {
            output_redirect = current;
            *current = '\0';
            current++;
        } else if (*current == '>' && current > cmd && *(current-1) == '2') {
            error_redirect = current-1;
            *(current-1) = '\0';
            current++;
        } else {
            current++;
        }
    }
    
    // Handle input redirection
    if (input_redirect != NULL) {
        char *filename = trim(input_redirect + 1);
        // Check if the filename has any redirection characters
        char *end = strpbrk(filename, "<>");
        if (end != NULL) *end = '\0';
        filename = trim(filename);
        
        *in_fd = open(filename, O_RDONLY);
        if (*in_fd == -1) {
            perror("open input");
            return -1;
        }
    }
    
    // Handle output redirection
    if (output_redirect != NULL) {
        char *filename = trim(output_redirect + 1);
        // Check if the filename has any redirection characters
        char *end = strpbrk(filename, "<>");
        if (end != NULL) *end = '\0';
        filename = trim(filename);
        
        *out_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (*out_fd == -1) {
            perror("open output");
            return -1;
        }
    }
    
    // Handle error redirection
    if (error_redirect != NULL) {
        char *filename = trim(error_redirect + 2);  // Skip "2>"
        // Check if the filename has any redirection characters
        char *end = strpbrk(filename, "<>");
        if (end != NULL) *end = '\0';
        filename = trim(filename);
        
        *err_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (*err_fd == -1) {
            perror("open error");
            return -1;
        }
    }
    
    // Parse command arguments
    char *token = strtok(cmd, " \t");
    while (token != NULL && arg_count < 255) {
        args[arg_count++] = token;
        token = strtok(NULL, " \t");
    }
    args[arg_count] = NULL;
    
    return arg_count;
}

/**
 * @brief Execute a pipeline of commands
 * @param commands Array of command strings
 * @param num_commands Number of commands in the pipeline
 * @return Exit status of the last command
 */
int execute_pipeline(char **commands, int num_commands) {
    int status = 0;
    int pipes[num_commands-1][2];
    pid_t pids[num_commands];
    
    // Create all necessary pipes
    for (int i = 0; i < num_commands-1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return EXIT_FAILURE;
        }
    }
    
    // Launch all processes
    for (int i = 0; i < num_commands; i++) {
        pids[i] = fork();
        
        if (pids[i] == -1) {
            perror("fork");
            return EXIT_FAILURE;
        } else if (pids[i] == 0) {
            // Child process
            
            // Setup file descriptors for redirections
            int in_fd = STDIN_FILENO;
            int out_fd = STDOUT_FILENO;
            int err_fd = STDERR_FILENO;
            
            // Handle pipe input (from previous command)
            if (i > 0) {
                in_fd = pipes[i-1][0];
            }
            
            // Handle pipe output (to next command)
            if (i < num_commands-1) {
                out_fd = pipes[i][1];
            }
            
            // Close all unused pipe ends
            for (int j = 0; j < num_commands-1; j++) {
                if (j != i-1) close(pipes[j][0]);
                if (j != i) close(pipes[j][1]);
            }
            
            char *args[256];
            int arg_count = parse_command(commands[i], args, &in_fd, &out_fd, &err_fd);
            
            if (arg_count <= 0) {
                fprintf(stderr, "Failed to parse command or empty command\n");
                exit(EXIT_FAILURE);
            }
            
            // Set up redirections
            if (in_fd != STDIN_FILENO) {
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }
            if (out_fd != STDOUT_FILENO) {
                dup2(out_fd, STDOUT_FILENO);
                close(out_fd);
            }
            if (err_fd != STDERR_FILENO) {
                dup2(err_fd, STDERR_FILENO);
                close(err_fd);
            }
            
            // Execute the command
            execvp(args[0], args);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }
    
    // Parent process
    // Close all pipe ends
    for (int i = 0; i < num_commands-1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    // Wait for all child processes
    for (int i = 0; i < num_commands; i++) {
        if (waitpid(pids[i], &status, 0) == -1) {
            perror("waitpid");
            return EXIT_FAILURE;
        }
    }
    
    return WEXITSTATUS(status);
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
        if (strncmp(trimmed_input, "cd", 2) == 0 && (trimmed_input[2] == ' ' || trimmed_input[2] == '\t' || trimmed_input[2] == '\0')) {
            char *token = strtok(trimmed_input, " \t");
            token = strtok(NULL, " \t");
            if (token != NULL) {
                if (chdir(token) != 0) {
                    perror("chdir");
                }
            } else {
                fprintf(stderr, "cd: missing argument\n");
            }
            continue;
        }

        // Check for pipes
        char *command_copy = strdup(trimmed_input);
        if (command_copy == NULL) {
            perror("strdup");
            exit(EXIT_FAILURE);
        }
        
        // Count the number of pipes
        int num_commands = 1;
        for (char *c = command_copy; *c != '\0'; c++) {
            if (*c == '|') num_commands++;
        }
        
        // Split the input by pipes
        char *commands[num_commands];
        char *cmd_token = strtok(command_copy, "|");
        int cmd_index = 0;
        
        while (cmd_token != NULL && cmd_index < num_commands) {
            commands[cmd_index++] = trim(cmd_token);
            cmd_token = strtok(NULL, "|");
        }
        
        // Execute the pipeline
        execute_pipeline(commands, num_commands);
        
        // Free the duplicated string
        free(command_copy);
    }

    return 0;
}