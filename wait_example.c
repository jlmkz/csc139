#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/**
 * This program demonstrates the use of the wait() system call.
 * The parent process forks a child and then waits for it to complete.
 * It also shows what happens when a process with no children calls wait().
 */

int main(int argc, char *argv[]) {
    printf("PARENT (pid:%d)\n", getpid());
    pid_t pid = fork();

    if (pid < 0) {
        // fork failed
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (pid == 0) { // child process
        printf("CHILD (pid:%d): Executing child process.\n", getpid());

        // Let's try to wait in the child.
        // Since this child has no children of its own, wait() will fail.
        printf("CHILD (pid:%d): Trying to call wait()...\n", getpid());
        pid_t wait_return = wait(NULL);

        if (wait_return == -1) {
            fprintf(stderr, "CHILD: wait() failed.\n");
        }

        printf("CHILD (pid:%d): Exiting now.\n", getpid());
        exit(0); // Child exits
    } else { // parent process
        printf("PARENT (pid:%d): Waiting for child (pid:%d) to finish.\n", getpid(), pid);

        pid_t terminated_pid = wait(NULL);

        printf("PARENT (pid:%d): Child process (pid:%d) has finished.\n", getpid(), terminated_pid);
        printf("PARENT (pid:%d): The return value of wait() was %d, which is the child's PID.\n", getpid(), terminated_pid);
    }

    return 0;
}