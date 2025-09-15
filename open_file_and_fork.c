#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    // Open a file for appending. fopen() with "a" mode creates the file
    // if it does not exist and positions the stream at the end of the file.
    FILE *fp = fopen("dummy.txt", "a");

    if (fp == NULL) {
        fprintf(stderr, "fopen failed");
        exit(1);
    }

    // We use fflush(stdout) to ensure the parent's message prints before any potential child output.
    printf("PARENT (pid:%d): Opened file dummy.txt.\n", (int)getpid());
    fflush(stdout);

    pid_t pid = fork();

    if (pid < 0) {
        // fork failed
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (pid == 0) {
        // child process
        printf("CHILD (pid:%d): Inherited file pointer.\n", (int)getpid());

        // Write to the file
        fprintf(fp, "\nThis line was written by the child.");
        printf("CHILD (pid:%d): Wrote to the file.\n", (int)getpid());

        fclose(fp); // Child closes its copy of the file stream
        exit(0);
    } else {
        // parent process
        // Write to the file
        fprintf(fp, "\nThis line was written by the parent.");
        printf("PARENT (pid:%d): Wrote to the file.\n", (int)getpid());

        // Wait for the child to finish
        wait(NULL);
        printf("PARENT (pid:%d): Child has finished.\n", (int)getpid());

        fclose(fp); // Parent closes its copy of the file stream
    }

    printf("Final check: You can now inspect the contents of dummy.txt\n");

    return 0;
}
