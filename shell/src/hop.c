//############## LLM Generated Code Begins ###############
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "hop.h"
#include "paths.h" // Required for the global 'prev_dir' and 'home_dir'

void hop(int argc, char **argv, int* hopconverted) 
{
    // Handle 'hop' with no arguments to go home.
    if (argc == 1) {
        char current_dir[1024];
        if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
            perror("hop: getcwd");
            return;
        }
        if (home_dir[0] == '\0') {
            fprintf(stderr, "hop: HOME not set\n");
            return;
        }
        if (chdir(home_dir) == 0) {
            strncpy(prev_dir, current_dir, sizeof(prev_dir) - 1);
            prev_dir[sizeof(prev_dir) - 1] = '\0';
            *hopconverted = 1;
        } else {
            printf("No such directory!\n");
        }
        return;
    }

    // Process each argument sequentially.
    for (int i = 1; i < argc; i++) {
        char current_dir[1024];
        if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
            perror("hop: getcwd");
            return; // Stop if we can't determine the current path.
        }

        char *target_dir = NULL;
        char *current_arg = argv[i];

        if (strcmp(current_arg, "-") == 0) {
            if (prev_dir[0] == '\0') {
                fprintf(stderr, "hop: no previous directory to switch to\n");
                return; // Stop on error
            }
            target_dir = prev_dir;
        } else if (strcmp(current_arg, "~") == 0) {
            if (home_dir[0] == '\0') {
                fprintf(stderr, "hop: HOME not set\n");
                return; // Stop on error
            }
            target_dir = home_dir;
        } else {
            target_dir = current_arg;
        }

        // Attempt to change directory for the current argument.
        if (chdir(target_dir) == 0) {
            // If successful, update the previous directory.
            strncpy(prev_dir, current_dir, sizeof(prev_dir) - 1);
            prev_dir[sizeof(prev_dir) - 1] = '\0';
            *hopconverted = 1; // Signal that a directory change happened.
        } else {
            // If any part of the path fails, print an error and stop.
            printf("No such directory!\n");
            return;
        }
    }
}

//############## LLM Generated Code Ends ###############