#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/**
 * This program demonstrates the use of the waitpid() system call.
 * The parent process forks a child and then waits for that specific child
 * to complete, retrieving its exit status.
 */

int main(int argc, char *argv[]) {
    printf("PARENT (pid:%d): Starting program.\n", getpid());

    pid_t pid = fork();

    if (pid < 0) { // fork failed
        fprintf(stderr, "fork failed\n");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { // child process
        printf("CHILD (pid:%d): Executing child process.\n", getpid());
        printf("CHILD (pid:%d): Exiting now.\n", getpid());
        exit(0);
    } else { // parent process
        printf("PARENT (pid:%d): Waiting specifically for child (pid:%d) to finish.\n", getpid(), pid);

        // waitpid() can wait for a specific child process.
        // 1st arg: The PID of the child to wait for.
        // 2nd arg: We pass NULL because we don't need the exit status here.
        // 3rd arg: Options (0 means wait blockingly).
        pid_t terminated_pid = waitpid(pid, NULL, 0);

        printf("PARENT (pid:%d): Child process (pid:%d) has finished.\n", getpid(), terminated_pid);
    }

    return 0;
}