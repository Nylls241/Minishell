#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Main function of the shell
 * @return 0 if everything goes well
 */
int main() {
    char input[1024]; ///< Buffer to store user input

    while (1) {
        // Display the command prompt
        printf("MyShell%% ");

        // Read user input
        if (fgets(input, sizeof(input), stdin) == NULL) {
            // Exit if CTRL+D is detected (EOF)
            printf("\nGoodbye!\n");
            break;
        }

        // Remove the newline character added by fgets
        input[strcspn(input, "\n")] = '\0';

        // Exit the program if the user enters "exit"
        if (strcmp(input, "exit") == 0) {
            printf("Goodbye!\n");
            break;
        }

        // Display the text entered by the user
        printf("%s\n", input);
    }

    return 0;
}