//############## LLM Generated Code Begins ###############
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include "prompt.h"
#include "paths.h" // Include for home_dir

// Extern declaration to access the global variable from paths.c
extern char home_dir[MAX_PATH];

void print_prompt()
{
    char *username = getenv("USER");
    if (!username) {
        username = "user";
    }
    char hostname[HOST_NAME_MAX + 1];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        strncpy(hostname, "localhost", sizeof(hostname));
        hostname[sizeof(hostname)-1] = '\0';
    }
    
    char current[PATH_MAX];
    if (getcwd(current, sizeof(current)) == NULL) {
        perror("getcwd in prompt");
        return;
    }
    
    char display_path[PATH_MAX];

    // Check if home_dir (startup directory) is a prefix of the current directory.
    if (home_dir[0] != '\0' && strncmp(current, home_dir, strlen(home_dir)) == 0)
    {
        snprintf(display_path, sizeof(display_path), "~%s", current + strlen(home_dir));
    }
    else
    {
        strncpy(display_path, current, sizeof(display_path));
        display_path[sizeof(display_path) - 1] = '\0';
    }

    printf("<%s@%s:%s> ", username, hostname, display_path);
}

//############## LLM Generated Code Ends ###############