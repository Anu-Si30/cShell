//############## LLM Generated Code Begins ###############
#ifndef LOGS_H
#define LOGS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// Load history from persistent file at startup
void load_log(void);

// Adds a command to the log (in-memory + persistent file)
void logging(const char *cmd);

// Prints the log (oldest -> newest)
void printlog(void);

// Clears the log (in-memory + persistent file)
void purgelog(void);

// Gets a command from the log (1-indexed, newest -> oldest)
const char* get_log_command(int index);

#endif
//############## LLM Generated Code Ends ###############
