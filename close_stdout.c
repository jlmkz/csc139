#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/**
 * This program demonstrates what happens when a process closes its
 * standard output file descriptor and then tries to print to it.
 */

int main(int argc, char *argv[]) {
    pid_t pid = fork();

    if (pid < 0) { // forked failed
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (pid == 0) { // child process
        printf("CHILD (pid:%d): This line should appear on your terminal.\n", getpid());
        printf("CHILD (pid:%d): Now closing standard output (STDOUT_FILENO).\n", getpid());

        // Ensure the above messages are written out before we close the descriptor.
        fflush(stdout);
        
        // Now, close the standard output file descriptor (value is 1).
        close(STDOUT_FILENO);

        // This printf will attempt to write to the 'stdout' stream.
        // The underlying write() system call will fail because the descriptor is closed.
        int ret = printf("CHILD (pid:%d): This line should NOT appear.\n", getpid());

        // Check the return value of printf(). On error, it returns a negative value.
        if (ret < 0) {
            // We can't print to stdout, but stderr (file descriptor 2) should still be open.
            fprintf(stderr, "CHILD (pid:%d): printf() failed as expected because STDOUT is closed.\n", getpid());
        }

        exit(0);
    } else { // parent process
        printf("PARENT (pid:%d): Created child (pid:%d).\n", getpid(), pid);
        wait(NULL); // Wait for the child to finish
        printf("PARENT (pid:%d): Child has finished. Check the output carefully.\n", getpid());
    }

    return 0;
}