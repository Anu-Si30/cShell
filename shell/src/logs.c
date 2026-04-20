//############## LLM Generated Code Begins ###############
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_HISTORY 15
#define LOG_FILE ".myshell_history"

static char history[MAX_HISTORY][1024];
static int history_count = 0;

// Load history from file at startup
void load_log() {
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", getenv("HOME"), LOG_FILE);

    FILE *f = fopen(path, "r");
    if (!f) return; // no history yet

    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = '\0'; // strip newline
        if (history_count < MAX_HISTORY) {
            strcpy(history[history_count++], line);
        } else {
            for (int i = 1; i < MAX_HISTORY; i++)
                strcpy(history[i - 1], history[i]);
            strcpy(history[MAX_HISTORY - 1], line);
        }
    }
    fclose(f);
}

void logging(const char *cmd) {
    if (!cmd || *cmd == '\0') return;        
    if (strncmp(cmd, "log", 3) == 0) return; 
    if (history_count > 0 && strcmp(history[history_count - 1], cmd) == 0) return;

    if (history_count < MAX_HISTORY) {
        strcpy(history[history_count++], cmd);
    } else {
        for (int i = 1; i < MAX_HISTORY; i++)
            strcpy(history[i - 1], history[i]);
        strcpy(history[MAX_HISTORY - 1], cmd);
    }

    // Append to file
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", getenv("HOME"), LOG_FILE);
    FILE *f = fopen(path, "a");
    if (f) {
        fprintf(f, "%s\n", cmd);
        fclose(f);
    }
}


void printlog() {
    for (int i = 0; i < history_count; i++) {
        printf("%s\n", history[i]);
    }
}

void purgelog() {
    history_count = 0;
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", getenv("HOME"), LOG_FILE);
    FILE *f = fopen(path, "w"); 
    if (f) fclose(f);
}

const char* get_log_command(int index) {
    if (index < 1 || index > history_count) {
        printf("Invalid log index\n");
        return NULL;
    }
    int idx = history_count - index; 
    return history[idx];
}
//############## LLM Generated Code Ends ###############