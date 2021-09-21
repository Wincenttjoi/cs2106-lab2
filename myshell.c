/**
 * CS2106 AY21/22 Semester 1 - Lab 2
 *
 * This file contains function definitions. Your implementation should go in
 * this file.
 */

#include "myshell.h"
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#define STATUS_RUNNING -1
#define STATUS_TERMINATED -2
#define PROCESS_SUCESS -3
#define PROCESS_FAIL -4

int PID_arr[MAX_PROCESSES];
int pid_index;

int PID_status[MAX_PROCESSES];

char *commands[MAX_PROCESSES];

void my_init(void)
{
    // Initialize what you need here
    pid_index = 0;
}

void print_string_array(char **array, int length)
{
    printf("======= PRINT STRING ARRAY =======\n");
    for (int i = 0; i < length; i++)
    {
        printf("INDEX: %d | VALUE: %s\n", i, array[i]);
    }
    printf("======= /PRINT STRING ARRAY =======\n");
}

void printInfo()
{
    for (int i = 0; i < pid_index; i++)
    {
        int child_pid = PID_arr[i];
        int child_stats = PID_status[i];
        // Process is running in the background
        int status = 1;
        if (child_stats == STATUS_RUNNING)
        {
            waitpid(child_pid, &status, WNOHANG);
            if (WIFEXITED(status))
            {
                child_stats = WEXITSTATUS(status);
                printf("[%i] Exited %d\n", child_pid, child_stats);
            }
            else
            {
                printf("[%i] Running\n", child_pid);
            }
        }
        else if (child_stats == STATUS_TERMINATED)
        {
            waitpid(child_pid, &status, WNOHANG);
            if (WIFEXITED(status) || (WTERMSIG(status) == SIGTERM))
            {
                child_stats = WEXITSTATUS(status);
                printf("[%i] Exited %d\n", child_pid, child_stats);
            }
            else
            {
                printf("[%i] Terminating\n", child_pid);
            }
        }
        else
        {
            // Process has exited
            printf("[%i] Exited %d\n", child_pid, child_stats);
        }
    }
}

char **parseSingleCommand(char **source, int start, int end)
{
    int length = (end - start + 1) * sizeof(*source);
    char **dest = malloc(length);
    memcpy(dest, source + start, length);
    dest[end - start + 1] = '\0';
    return dest;
}

int processRedirection(int is_ambercent, char **tokens)
{
}

int processCommand(int is_ambercent, char **tokens, int num_tokens)
{
    // Check file exists, if not will return process fail
    if (access(tokens[0], F_OK) == -1)
    {
        printf("%s not found\n", tokens[0]);
        return PROCESS_FAIL;
    }

    int result = fork();
    if (result != 0)
    {
        // Parent code
        PID_arr[pid_index] = result;

        // Foreground
        if (is_ambercent == 0)
        {
            int status;
            waitpid(result, &status, 0);
            PID_status[pid_index] = WEXITSTATUS(status);
        }
        else
        {
            // Background
            printf("Child[%i] in background\n", result);
            PID_status[pid_index] = STATUS_RUNNING;
        }

        pid_index++;

        if (PID_status[pid_index - 1] == 0)
        {
            return PROCESS_SUCESS;
        }
        else
        {
            return PROCESS_FAIL;
        }
    }
    else
    {
        // Child code
        // Check for any redirection
        for (int i = 0; i < num_tokens - 2; i++)
        {
            char *file = tokens[i + 1];
            if (strcmp(tokens[i], "<") == 0)
            {

                if (access(tokens[i + 1], F_OK) == -1)
                {
                    printf("%s does not exist\n", tokens[i + 1]);
                    return PROCESS_FAIL;
                }
                int fd = open(file, O_RDONLY, S_IRUSR | S_IWUSR);
                dup2(fd, STDIN_FILENO);
                close(fd);
                tokens[i] = '\0';
            }
            else if (strcmp(tokens[i], ">") == 0)
            {
                int fd = open(file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
                dup2(fd, STDOUT_FILENO);
                close(fd);

                tokens[i] = '\0';
            }
            else if (strcmp(tokens[i], "2>") == 0)
            {
                int fd = open(file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
                dup2(fd, STDERR_FILENO);
                close(fd);

                tokens[i] = '\0';
            }
        }

        int isExecSuccessful = execvp(tokens[0], tokens);
        // Error handling
        if (isExecSuccessful == -1)
        {
            printf("%s not found\n", tokens[0]);
            exit(1);
        }
    }
}

void my_process_command(size_t num_tokens, char **tokens)
{

    if (strcmp(tokens[0], "info") == 0)
    {
        // DO INFO
        printInfo();
        return;
    }
    else if (strcmp(tokens[0], "wait") == 0)
    {
        int pid_wait_argument = atoi(tokens[1]);
        for (int i = 0; i < pid_index; i++)
        {
            if (pid_wait_argument == PID_arr[i] && PID_status[i] == STATUS_RUNNING)
            {
                int status;
                waitpid(PID_arr[i], &status, 0);
                PID_status[i] = WEXITSTATUS(status);
                break;
            }
        }
        return;
    }
    else if (strcmp(tokens[0], "terminate") == 0)
    {
        int pid_wait_argument = atoi(tokens[1]);
        for (int i = 0; i < pid_index; i++)
        {
            if (pid_wait_argument == PID_arr[i] && PID_status[i] == STATUS_RUNNING)
            {
                kill(PID_arr[i], SIGTERM);
                PID_status[i] = STATUS_TERMINATED;
                break;
            }
        }
        return;
    }
    else
    {
        // Program exists
        int is_ambercent = 0;
        if (strcmp(tokens[num_tokens - 2], "&") == 0)
        {
            is_ambercent = 1;
            tokens[num_tokens - 2] = '\0';
        }

        int prev = 0;
        int process_status = 0;
        for (int i = 0; i < num_tokens - 2; i++)
        {
            if (strcmp(tokens[i], "&&") == 0)
            {
                int length = i + 1 - prev;
                char **proc = parseSingleCommand(tokens, prev, i - 1);
                process_status = processCommand(0, proc, length);
                prev = i + 1;

                if (process_status == PROCESS_FAIL)
                {
                    // printf("%s failed\n", proc[0]);
                    break;
                }
            }
        }

        if (prev > 0 && process_status != PROCESS_FAIL)
        {
            int length = num_tokens - prev;
            char **proc = parseSingleCommand(tokens, prev, num_tokens - 2);
            processCommand(0, proc, length);
        }

        if (prev == 0)
        {
            processCommand(is_ambercent, tokens, num_tokens);
        }
    }
}

void my_quit(void)
{
    // Clean up function, called after "quit" is entered as a user command
    for (int i = 0; i < pid_index; i++)
    {
        kill(PID_arr[i], SIGTERM);
        waitpid(PID_arr[i], NULL, 0);
    }
    printf("Goodbye!\n");
}
