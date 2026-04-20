//############## LLM Generated Code Begins ###############
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <termios.h> 

#include "prompt.h"
#include "parser.h"
#include "hop.h"
#include "reveal.h"
#include "paths.h"
#include "logs.h"
#include "execute.h"
#include "jobs.h"

// Forward-declarations for your custom commands
void hop(int argc, char **argv, int *hopconverted);
void reveal(int argc, char **argv, int hop_flag, int *hopconverted);
void printlog();
void purgelog();
const char* get_log_command(int idx);
void logging(const char *line);
void load_log();
void print_prompt();

// Global flag for handling Ctrl+C at a blank prompt
volatile sig_atomic_t sigint_flag = 0;

// Minimal handler for Ctrl+C
void sigint_handler(int sig) {
    (void)sig;
    if (get_fg_pid() > 0) {
        kill(-get_fg_pid(), SIGINT);
    } else {
        sigint_flag = 1;
    }
}

// Minimal handler for Ctrl+Z
void sigtstp_handler(int sig) {
    (void)sig;
    if (get_fg_pid() > 0) {
        kill(-get_fg_pid(), SIGTSTP);
    }
}

void setup_signal_handlers() {
    struct sigaction sa_int, sa_tstp;

    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa_int, NULL);

    sa_tstp.sa_handler = sigtstp_handler;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &sa_tstp, NULL);
}

int custom(int argc, char **argv, int *hopconverted, int *hop_flag) {
    if (argc == 0) {
        return 0;
    }
    
    if (strcmp(argv[0], "reveal") == 0) {
        reveal(argc, argv, *hop_flag, hopconverted);
        return 1;
    } else if (strcmp(argv[0], "log") == 0) {
        if (argc == 1) printlog();
        else if (argc == 2 && strcmp(argv[1], "purge") == 0) purgelog();
        else if (argc == 3 && strcmp(argv[1], "execute") == 0) {
            int idx = atoi(argv[2]);
            const char *cmd = get_log_command(idx);
            if (cmd) {
                char temp[1024];
                strncpy(temp, cmd, sizeof(temp) - 1);
                temp[sizeof(temp) - 1] = '\0';
                execute_command(temp, hopconverted, hop_flag);
            }
        } else {
            printf("Invalid Syntax\n");
        }
        return 1;
    } else if (strcmp(argv[0], "activities") == 0) {
        list_activities();
        return 1;
    } else if (strcmp(argv[0], "ping") == 0) {
        if (argc != 3) {
            printf("Invalid syntax!\n");
            return 1;
        }
        char *endptr_pid, *endptr_sig;
        long pid_val = strtol(argv[1], &endptr_pid, 10);
        long sig_val = strtol(argv[2], &endptr_sig, 10);
        if (*endptr_pid != '\0' || *endptr_sig != '\0') {
            printf("Invalid syntax!\n");
            return 1;
        }
        pid_t pid = (pid_t)pid_val;
        int signal_number = (int)sig_val;
        int actual_signal = signal_number % 32;
        if (kill(pid, 0) == -1 && errno == ESRCH) {
            printf("No such process found\n");
        } else {
            if (kill(pid, actual_signal) == 0) {
                printf("Sent signal %d to process with pid %d\n", signal_number, pid);
            } else {
                perror("kill");
            }
        }
        return 1;
    } else if (strcmp(argv[0], "fg") == 0) {
        struct job_t* job = NULL;
        if (argc == 1) {
            job = get_most_recent_job();
        } else {
            job = get_job_by_jid(atoi(argv[1]));
        }

        if (job == NULL) {
            printf("fg: No such job\n");
            return 1;
        }

        printf("%s\n", job->cmdline);
        pid_t pid = job->pid;
        char cmdline[256];
        strcpy(cmdline, job->cmdline);
        
        // Give terminal control to the job's process group
        if (tcsetpgrp(STDIN_FILENO, pid) < 0) {
            perror("tcsetpgrp");
            return 1;
        }
        
        if (job->state == STATE_STOPPED) {
            kill(-pid, SIGCONT);
        }

        delete_job(pid);
        set_fg_pid(pid);

        int status;
        waitpid(pid, &status, WUNTRACED);
        
        // Take terminal control back for the shell
        if (tcsetpgrp(STDIN_FILENO, getpgrp()) < 0) {
            perror("tcsetpgrp");
        }

        if (WIFSTOPPED(status)) {
            add_job(pid, cmdline, STATE_STOPPED);
            struct job_t* stopped_job = get_job_by_pid(pid);
            if (stopped_job) {
                printf("\n[%d] Stopped %s\n", stopped_job->jid, stopped_job->cmdline);
            }
        }
        set_fg_pid(0);
        return 1;
    } else if (strcmp(argv[0], "bg") == 0) {
        struct job_t* job = NULL;
        if (argc == 1) {
            job = get_most_recent_job();
        } else {
            job = get_job_by_jid(atoi(argv[1]));
        }

        if (job == NULL) {
            printf("bg: No such job\n");
            return 1;
        }
        
        if (job->state == STATE_RUNNING) {
            printf("bg: job %d already in background\n", job->jid);
            return 1;
        }

        kill(-job->pid, SIGCONT);
        job->state = STATE_RUNNING;
        printf("[%d] %s &\n", job->jid, job->cmdline);
        return 1;
    }

    return 0; // Not a built-in command
}

int main() {
    int hopco = 0;
    int* hopconverted = &hopco;
    int hop_flag = 0;
    char *line = NULL;
    size_t len = 0;

    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    setup_signal_handlers();
    init_paths();
    load_log();
    init_jobs();

    while (1) {
        if (sigint_flag) {
            printf("\n");
            sigint_flag = 0;
        }

        reap_background_jobs();
        print_prompt();
        ssize_t nread = getline(&line, &len, stdin);

        if (nread == -1) {
            if (feof(stdin)) { // Handle EOF (Ctrl+D)
                printf("logout\n");
                kill_all_jobs();
                break;
            }
            continue; // Handle other errors like EINTR
        }

        line[strcspn(line, "\n")] = '\0';

        if (strcmp(line, "exit") == 0) {
            break;
        }
        
        if (strlen(line) == 0) {
            continue;
        }

        logging(line);
        execute_command(line, hopconverted, &hop_flag);
    }

    free(line);
    return 0;
}

//############## LLM Generated Code Ends ###############