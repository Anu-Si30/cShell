//############## LLM Generated Code Begins ###############
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include "reveal.h"
#include "paths.h"
#include "hop.h"

// Comparator for qsort (lexicographic ASCII order)
static int cmp_strings(const void *a, const void *b) {
    const char *s1 = *(const char **)a;
    const char *s2 = *(const char **)b;
    return strcmp(s1, s2);
}

// Main reveal function
void reveal(int argc, char **argv, int hop_flag, int* hopconverted) {
    (void)hop_flag;
    (void)hopconverted;
    int show_all = 0;   // -a flag
    int line_mode = 0;  // -l flag
    char *target_dir = NULL;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-") == 0) {
            // Previous directory case
            if (prev_dir[0] == '\0') {
                printf("No such directory!\n");
                return;
            }
            target_dir = prev_dir;
        }
        else if (strcmp(argv[i], "~") == 0) {
            // Home directory
            if (home_dir[0] == '\0') {
                printf("No such directory!\n");
                return;
            }
            target_dir = home_dir;
        }
        else if (argv[i][0] == '-') {
            // Handle flags like -a and -l
            for (int j = 1; argv[i][j] != '\0'; j++) {
                if (argv[i][j] == 'a') show_all = 1;
                else if (argv[i][j] == 'l') line_mode = 1;
                else {
                    printf("reveal: Invalid Syntax!\n");
                    return;
                }
            }
        }
        else {
            // Path argument
            if (target_dir != NULL) {
                printf("reveal: Invalid Syntax!\n");
                return;
            }
            target_dir = argv[i];
        }
    }

    // Default directory: current working dir
    char cwd[MAX_PATH];
    if (target_dir == NULL) {
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd");
            return;
        }
        target_dir = cwd;
    }

    // Open directory
    DIR *dir = opendir(target_dir);
    if (!dir) {
        printf("No such directory!\n");
        return;
    }

    // Collect entries
    struct dirent *entry;
    char **names = NULL;
    size_t count = 0;

    while ((entry = readdir(dir)) != NULL) {
        // Skip hidden unless -a
        if (!show_all && entry->d_name[0] == '.')
            continue;

        names = realloc(names, (count + 1) * sizeof(char *));
        if (!names) {
            perror("realloc");
            closedir(dir);
            return;
        }
        names[count] = strdup(entry->d_name);
        if (!names[count]) {
            perror("strdup");
            closedir(dir);
            return;
        }
        count++;
    }
    closedir(dir);

    // Sort lexicographically
    qsort(names, count, sizeof(char *), cmp_strings);

    // Print results
    for (size_t i = 0; i < count; i++) {
        if (line_mode)
            printf("%s\n", names[i]);
        else
            printf("%s ", names[i]);
        free(names[i]);
    }
    free(names);

    if (!line_mode)
        printf("\n");
}

//############## LLM Generated Code Ends ###############