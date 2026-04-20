//############## LLM Generated Code Begins ###############
#include "execute.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <termios.h>

#include "parser.h"
#include "hop.h"
#include "jobs.h"

// Forward declarations for functions in this file
void execute_pipeline(char *line, int background, const char* original_cmd, int *hopconverted, int *hop_flag);
void execute_single_command(char *line, int background, const char* original_cmd, int *hopconverted, int *hop_flag);
void parse_line_simple(char* line, char** argv);

// Forward declarations for functions from other files
int custom(int argc, char **argv, int *hopconverted, int *hop_flag);
int valid_command(const char *line);

void execute_command(char *line, int *hopconverted, int *hop_flag)
{
    char original_cmd[1024];
    strncpy(original_cmd, line, sizeof(original_cmd) - 1);
    original_cmd[sizeof(original_cmd) - 1] = '\0';
    
    char *end = original_cmd + strlen(original_cmd) - 1;
    while (end >= original_cmd && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '&'))
    {
        *end-- = '\0';
    }

    int background = 0;
    char *ampersand = strrchr(line, '&');
    if (ampersand != NULL)
    {
        char *check = ampersand + 1;
        while (*check != '\0' && (*check == ' ' || *check == '\t' || *check == '\n'))
        {
            check++;
        }
        if (*check == '\0')
        {
            background = 1;
            *ampersand = '\0';
        }
    }
    
    if (strchr(line, ';'))
    {
        char *line_copy = strdup(line);
        char *cmd;
        char *rest = line_copy;
        while ((cmd = strtok_r(rest, ";", &rest)))
        {
            execute_command(cmd, hopconverted, hop_flag);
        }
        free(line_copy);
        return;
    }

    if (strchr(line, '|'))
    {
        execute_pipeline(line, background, original_cmd, hopconverted, hop_flag);
    }
    else
    {
        execute_single_command(line, background, original_cmd, hopconverted, hop_flag);
    }
}

void execute_single_command(char *line, int background, const char* original_cmd, int *hopconverted, int *hop_flag)
{
    if (!valid_command(line))
    {
        printf("Invalid Syntax\n");
        return;
    }

    char *argv[1024];
    int argc = 0;
    char *input_file = NULL, *output_file = NULL;
    int append_mode = 0;

    char *line_copy = strdup(line);
    char *token = strtok(line_copy, " \t\n");

    while (token)
    {
        if (strcmp(token, "<") == 0) { token = strtok(NULL, " \t\n"); input_file = token; }
        else if (strcmp(token, ">") == 0) { append_mode = 0; token = strtok(NULL, " \t\n"); output_file = token; }
        else if (strcmp(token, ">>") == 0) { append_mode = 1; token = strtok(NULL, " \t\n"); output_file = token; }
        else { argv[argc++] = token; }
        token = strtok(NULL, " \t\n");
    }
    argv[argc] = NULL;
    
    if (argc == 0)
    {
        free(line_copy);
        return;
    }

    if (strcmp(argv[0], "hop") == 0) {
        hop(argc, argv, hopconverted);
        free(line_copy);
        return;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
    }
    else if (pid == 0)
    {
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        setpgid(0, 0);

        if (background)
        {
            int fd_null = open("/dev/null", O_RDONLY);
            dup2(fd_null, STDIN_FILENO);
            close(fd_null);
        }
        if (input_file)
        {
            int fd_in = open(input_file, O_RDONLY);
            if (fd_in < 0) { perror(input_file); exit(1); }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }
        if (output_file)
        {
            int flags = O_WRONLY | O_CREAT | (append_mode ? O_APPEND : O_TRUNC);
            int fd_out = open(output_file, flags, 0644);
            if (fd_out < 0) {
                fprintf(stderr, "Unable to create file for writing\n");
                exit(1);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        if (custom(argc, argv, hopconverted, hop_flag))
        {
            exit(0);
        }
        else
        {
            execvp(argv[0], argv);
            fprintf(stderr, "Command not found!\n");
            exit(1);
        }
    }
    else
    {
        if (background)
        {
            add_job(pid, original_cmd, STATE_RUNNING);
        }
        else
        {
            set_fg_pid(pid);
            int status;
            waitpid(pid, &status, WUNTRACED);

            if (WIFSTOPPED(status)) {
                add_job(pid, original_cmd, STATE_STOPPED);
                struct job_t* stopped_job = get_job_by_pid(pid);
                if (stopped_job) {
                    printf("\n[%d] Stopped %s\n", stopped_job->jid, stopped_job->cmdline);
                }
            } else if (WIFSIGNALED(status) || WIFEXITED(status)) {
                delete_job(pid);
            }
            set_fg_pid(0);
        }
    }
    free(line_copy);
}

void execute_pipeline(char *line, int background, const char* original_cmd, int *hopconverted, int *hop_flag)
{
    pid_t pgid = fork();
    if (pgid < 0)
    {
        perror("fork");
        return;
    }

    if (pgid == 0)
    {
        setpgid(0, 0);
        char *cmds[128];
        int num_cmds = 0;
        char *line_copy = strdup(line);
        char* rest = line_copy;
        char* cmd;

        while ((cmd = strtok_r(rest, "|", &rest)))
        {
            cmds[num_cmds++] = cmd;
        }

        int pipes[num_cmds - 1][2];
        pid_t pids[num_cmds];
        for (int i = 0; i < num_cmds - 1; i++)
        {
            if (pipe(pipes[i]) < 0) { perror("pipe"); exit(1); }
        }

        for (int i = 0; i < num_cmds; i++)
        {
            pids[i] = fork();
            if (pids[i] == 0)
            {
                signal(SIGINT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);

                if (i > 0) dup2(pipes[i-1][0], STDIN_FILENO);
                if (i < num_cmds - 1) dup2(pipes[i][1], STDOUT_FILENO);
                
                for (int j = 0; j < num_cmds - 1; j++)
                {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }

                char *argv[1024];
                parse_line_simple(cmds[i], argv);
                
                int argc = 0;
                while (argv[argc] != NULL) argc++;
                
                if (custom(argc, argv, hopconverted, hop_flag))
                {
                    exit(0);
                }
                else
                {
                    execvp(argv[0], argv);
                    fprintf(stderr, "Command not found!\n");
                    exit(1);
                }
            }
            else
            {
                 setpgid(pids[i], pgid);
            }
        }

        for (int i = 0; i < num_cmds - 1; i++)
        {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
        for (int i = 0; i < num_cmds; i++) 
        {
            int status;
            waitpid(pids[i], &status, WUNTRACED);
        }
        free(line_copy);
        exit(0);
    }
    else
    {
        if (background)
        {
            add_job(pgid, original_cmd, STATE_RUNNING);
        }
        else
        {
            set_fg_pid(pgid);
            int status;
            waitpid(pgid, &status, WUNTRACED);

             if (WIFSTOPPED(status)) {
                add_job(pgid, original_cmd, STATE_STOPPED);
                struct job_t* stopped_job = get_job_by_pid(pgid);
                if (stopped_job) {
                    printf("\n[%d] Stopped %s\n", stopped_job->jid, stopped_job->cmdline);
                }
            } else if (WIFSIGNALED(status) || WIFEXITED(status)) {
                delete_job(pgid);
            }
            set_fg_pid(0);
        }
    }
}

void parse_line_simple(char* line, char** argv)
{
    int i = 0;
    char *token = strtok(line, " \t\n");
    while (token != NULL)
    {
        argv[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    argv[i] = NULL;
}


//############## LLM Generated Code Ends ###############