//############## LLM Generated Code Begins ###############
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "paths.h"

// Define the global variables here
char prev_dir[MAX_PATH] = "";
char home_dir[MAX_PATH] = ""; // This is the shell's startup directory

void init_paths(void) {
    // Set the shell's home directory to the current working directory at startup
    if (getcwd(home_dir, sizeof(home_dir)) == NULL) {
        perror("init_paths: getcwd");
        home_dir[0] = '\0'; // Ensure it's empty on failure
    }
    
    // prev_dir is intentionally left empty at the start.
    // It will only be populated after the first successful 'hop' command.
}

//############## LLM Generated Code Ends ###############