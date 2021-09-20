/**
 * CS2106 AY21/22 Semester 1 - Lab 2
 *
 * This file contains function definitions. Your implementation should go in
 * this file.
 */

#include "myshell.h"
#include <sys/wait.h>

int PID_arr[MAX_PROCESSES];
int pid_index;

int PID_status[MAX_PROCESSES];

void my_init(void) {
    // Initialize what you need here
    pid_index = 0;
}

void printInfo() {
    for (int i = 0; i < pid_index; i++) {
        int child_pid = PID_arr[i];
        int child_stats = PID_status[i];
        if (child_stats != -1) {
            // Process is running in the background
            if (child_stats == 0) {
                int status;
                waitpid(child_pid, &status, WNOHANG);
                if (WIFEXITED(status)) {
                    PID_status[i] = status;
                    printf("[%i] Exited %d\n", child_pid, status);
                } else {
                    printf("[%i] Running\n", child_pid);
                }

            } else {
                // Process has exited
                printf("[%i] Exited %d\n", child_pid, child_stats);
            }
        } else {
            printf("Error statement here");
        }
    }
}

void my_process_command(size_t num_tokens, char **tokens) {

    int is_ambercent = 0;
    if (strcmp(tokens[num_tokens - 2], "&") == 0) {
        is_ambercent = 1;
        tokens[num_tokens - 2] = '\0';
    }


    if (strcmp(tokens[0], "info") == 0) {
        // DO INFO
        // for (int i = 1; i < MAX_PROCESSES && isdigit(PID_arr[i]); i++) {
            printInfo();
        // }
        return;

    } else {
        int result = fork();
        if (result != 0) {
            // Parent code
            PID_arr[pid_index] = result;
            
            // Foreground
            if (is_ambercent == 0) {
                int status;
                waitpid(result, &status, 0);
                PID_status[pid_index] = status;
            }
            else {
            // Background
                printf("Child[%i] in background\n", result);
                PID_status[pid_index] = 0;
            }

            pid_index++;

        } else {
            // Child code
            int isExecSuccessful = execvp(tokens[0], tokens);
            if (isExecSuccessful == -1) {
                printf("%s not found\n", tokens[0]);
                exit(1);
            }

        }
    }


}

void my_quit(void) {
    // Clean up function, called after "quit" is entered as a user command

}