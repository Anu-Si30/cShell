//############## LLM Generated Code Begins ###############
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include "jobs.h"

// The global table to store job information
struct job_t jobs_table[MAX_JOBS];
int next_jid = 1;

// Global variable to track the current foreground process PID
volatile sig_atomic_t fg_pid = 0;

void init_jobs() {
    for (int i = 0; i < MAX_JOBS; i++) {
        jobs_table[i].pid = 0;
        jobs_table[i].jid = 0;
        jobs_table[i].state = STATE_UNDEFINED;
        jobs_table[i].cmdline[0] = '\0';
    }
}

int add_job(pid_t pid, const char* cmdline, JobState state) {
    if (pid < 1) return 0;
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs_table[i].pid == 0) {
            jobs_table[i].pid = pid;
            jobs_table[i].state = state;
            jobs_table[i].jid = next_jid++;
            strncpy(jobs_table[i].cmdline, cmdline, sizeof(jobs_table[i].cmdline) - 1);
            jobs_table[i].cmdline[sizeof(jobs_table[i].cmdline) - 1] = '\0';
            
            if (state == STATE_RUNNING) {
                printf("[%d] %d\n", jobs_table[i].jid, jobs_table[i].pid);
            }
            return 1;
        }
    }
    printf("Error: Max jobs reached\n");
    return 0;
}

int delete_job(pid_t pid) {
    if (pid < 1) return 0;
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs_table[i].pid == pid) {
            jobs_table[i].pid = 0;
            jobs_table[i].jid = 0;
            jobs_table[i].state = STATE_UNDEFINED;
            jobs_table[i].cmdline[0] = '\0';
            return 1;
        }
    }
    return 0;
}

struct job_t* get_job_by_pid(pid_t pid) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs_table[i].pid == pid) {
            return &jobs_table[i];
        }
    }
    return NULL;
}

void reap_background_jobs() {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (pid == get_fg_pid()) continue;
        
        struct job_t* job = get_job_by_pid(pid);
        if (job) {
            if (WIFEXITED(status)) {
                printf("\n%s with pid %d exited normally\n", job->cmdline, pid);
            } else if (WIFSIGNALED(status)) {
                printf("\n%s with pid %d exited abnormally\n", job->cmdline, pid);
            }
            delete_job(pid);
        }
    }
}

struct activity_entry {
    pid_t pid;
    char command[256];
    char state_str[20];
};

// This is the corrected function
int compare_activities(const void *a, const void *b) {
    struct activity_entry *entry_a = (struct activity_entry *)a;
    struct activity_entry *entry_b = (struct activity_entry *)b;
    return strcmp(entry_a->command, entry_b->command);
}

void list_activities() {
    struct activity_entry sorted_list[MAX_JOBS];
    int job_count = 0;

    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs_table[i].pid != 0) {
            sorted_list[job_count].pid = jobs_table[i].pid;
            strcpy(sorted_list[job_count].command, jobs_table[i].cmdline);

            char proc_path[256];
            sprintf(proc_path, "/proc/%d/stat", jobs_table[i].pid);
            FILE *f = fopen(proc_path, "r");

            if (f) {
                char state_char;
                fscanf(f, "%*d %*s %c", &state_char);
                fclose(f);
                if (state_char == 'R' || state_char == 'S') {
                    strcpy(sorted_list[job_count].state_str, "Running");
                } else if (state_char == 'T') {
                    strcpy(sorted_list[job_count].state_str, "Stopped");
                } else {
                    strcpy(sorted_list[job_count].state_str, "Running");
                }
            } else {
                 strcpy(sorted_list[job_count].state_str, "Terminated");
            }
            job_count++;
        }
    }

    qsort(sorted_list, job_count, sizeof(struct activity_entry), compare_activities);

    for (int i = 0; i < job_count; i++) {
        printf("[%d] : %s - %s\n", sorted_list[i].pid, sorted_list[i].command, sorted_list[i].state_str);
    }
}

pid_t get_fg_pid() {
    return fg_pid;
}

void set_fg_pid(pid_t pid) {
    fg_pid = pid;
}

void kill_all_jobs() {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs_table[i].pid != 0) {
            kill(jobs_table[i].pid, SIGKILL);
        }
    }
}

struct job_t* get_job_by_jid(int jid) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs_table[i].pid != 0 && jobs_table[i].jid == jid) {
            return &jobs_table[i];
        }
    }
    return NULL;
}

struct job_t* get_most_recent_job() {
    int max_jid = -1;
    struct job_t* recent_job = NULL;
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs_table[i].pid != 0 && jobs_table[i].jid > max_jid) {
            max_jid = jobs_table[i].jid;
            recent_job = &jobs_table[i];
        }
    }
    return recent_job;
}
//############## LLM Generated Code Ends ###############