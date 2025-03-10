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
        printf("MonShell%% ");

        // Read user input
        if (fgets(input, sizeof(input), stdin) == NULL) {
            // Exit if CTRL+D is detected (EOF)
            printf("\nGoodbye!\n");
            break;
        }

        // Remove the newline character added by fgets
        input[strcspn(input, "\n")] = '\0';

        // Display the text entered by the user
        printf("%s\n", input);
    }

    return 0;
}