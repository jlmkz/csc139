#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

/*
 * This program demonstrates a pipe between two child processes.
 *
 * It creates two children:
 * 1. The first child (writer) writes a hardcoded message directly to the
 *    pipe's write file descriptor using the write() system call.
 * 2. The second child (reader) reads data from the pipe's read file
 *    descriptor using the read() system call and prints it to stdout.
 *
 * The parent process creates the pipe, forks the children, closes its
 * pipe ends, and waits for the children to complete.
 */

int main() {
    pid_t writer_pid, reader_pid;
    char buffer[128];

    // Create the pipe
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // --- Fork the Writer Child ---
    writer_pid = fork();
    if (writer_pid < 0) {
        perror("fork (writer)");
        exit(EXIT_FAILURE);
    }

    if (writer_pid == 0) {
        // --- Writer Child Logic ---
        fprintf(stderr, "WRITER (pid:%d): Process started.\n", getpid());

        // Close the unused read end of the pipe
        close(pipefd[0]);

        const char *message = "Hello from the writer process!\nThis data is sent through a pipe.\nHave a nice day!\n";
        fprintf(stderr, "WRITER (pid:%d): Sending message...\n", getpid());

        // Write the message directly to the pipe's write file descriptor
        write(pipefd[1], message, strlen(message));

        // Close the write end. This is crucial; it sends an EOF signal to the reader.
        close(pipefd[1]);
        fprintf(stderr, "WRITER (pid:%d): Message sent and pipe closed. Exiting.\n", getpid());
        exit(EXIT_SUCCESS);
    }

    // --- Fork the Reader Child ---
    reader_pid = fork();
    if (reader_pid < 0) {
        perror("fork (reader)");
        exit(EXIT_FAILURE);
    }

    if (reader_pid == 0) {
        // --- Reader Child Logic ---
        fprintf(stderr, "READER (pid:%d): Process started.\n", getpid());

        // Close the unused write end of the pipe
        close(pipefd[1]);

        fprintf(stderr, "READER (pid:%d): Waiting to read from pipe...\n", getpid());

        // Read from the pipe in a loop until EOF (read() returns 0)
        int bytes_read;
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            // Null-terminate the string before printing
            buffer[bytes_read] = '\0';
            // Print the received chunk of data to standard output
            printf("%s", buffer);
        }

        // Close the read end
        close(pipefd[0]);
        fprintf(stderr, "READER (pid:%d): Finished reading (EOF). Exiting.\n", getpid());
        exit(EXIT_SUCCESS);
    }

    // --- Parent Process Logic ---
    fprintf(stderr, "PARENT (pid:%d): Created writer (pid:%d) and reader (pid:%d).\n", getpid(), writer_pid, reader_pid);

    // Parent must close both its ends of the pipe.
    close(pipefd[0]);
    close(pipefd[1]);
    fprintf(stderr, "PARENT (pid:%d): Closed my ends of the pipe.\n", getpid());

    // Wait for both children to prevent zombies
    fprintf(stderr, "PARENT (pid:%d): Waiting for children to finish.\n", getpid());
    waitpid(writer_pid, NULL, 0);
    waitpid(reader_pid, NULL, 0);

    fprintf(stderr, "PARENT (pid:%d): Both children finished. Program complete.\n", getpid());

    return EXIT_SUCCESS;
}