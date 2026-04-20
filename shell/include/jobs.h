//############## LLM Generated Code Begins ###############
#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>

#define MAX_JOBS 16

// Defines the possible states a job can be in
typedef enum {
    STATE_UNDEFINED,
    STATE_RUNNING,
    STATE_STOPPED,
    STATE_DONE
} JobState;

// Structure to hold information about a single job
struct job_t {
    pid_t pid;
    int jid;
    JobState state;
    char cmdline[256];
};

// --- Function Declarations ---

// Job list management
void init_jobs();
int add_job(pid_t pid, const char* cmdline, JobState state);
int delete_job(pid_t pid);
struct job_t* get_job_by_pid(pid_t pid);

// Job state management
void reap_background_jobs();
void list_activities();
void kill_all_jobs();

// Foreground job tracking
pid_t get_fg_pid();
void set_fg_pid(pid_t pid);

// ✅ NEW: Helpers for fg and bg
struct job_t* get_job_by_jid(int jid);
struct job_t* get_most_recent_job();

#endif // JOBS_H
//############## LLM Generated Code Ends ###############