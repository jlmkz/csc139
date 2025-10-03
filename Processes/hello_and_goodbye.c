#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    // Create a pipe. pipefd[0] is for reading, pipefd[1] is for writing.
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        fprintf(stderr, "pipe failed");
        exit(1);
    }

    pid_t pid = fork();

    if (pid < 0) {
        // fork failed
        fprintf(stderr, "fork failed");
        exit(1);

    } else if (pid == 0) {
        // child process

        // The child will write, so it closes the read-end of the pipe.
        close(pipefd[0]);

        printf("hello\n");
        char signal[] = "green light";

        // After printing, signal the parent by writing a single byte to the pipe.
        write(pipefd[1], signal, sizeof(signal));

        // Close the write-end before exiting.
        close(pipefd[1]);
        exit(0);

    } else {
        // parent process

        // The parent will read, so it closes the write-end of the pipe.
        close(pipefd[1]);

        char buf;
        // The read() call will block until the child writes to the pipe.
        read(pipefd[0], &buf, 1);

        printf("goodbye\n");

        // Close the read-end after we're done.
        close(pipefd[0]);
    }

    return 0;
}