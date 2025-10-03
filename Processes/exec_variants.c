#define _GNU_SOURCE // For execvpe()
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

/**
 * This program demonstrates the different variants of the exec() family of calls.
 * It forks a child process, which then executes '/bin/ls -l' using a method
 * specified by a command-line argument.
 *
 * How to compile and run:
 * 1. Compile: gcc -o exec_variants exec_variants.c
 * 2. Run with a variant, for example: ./exec_variants execlp
 *
 * You can try any of the following variants:
 * execl, execle, execlp, execv, execvp, execvpe
 */

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <exec_variant>\n", argv[0]);
        fprintf(stderr, "Variants: execl, execle, execlp, execv, execvp, execvpe\n");
        exit(1);
    }

    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "exec failed\n");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { // child Process
        // This code block will be replaced by the 'ls' program upon a successful exec() call.

        printf("Child executing /bin/ls using: %s\n", argv[1]);

        if (strcmp(argv[1], "execl") == 0) {
            execl("/bin/ls", "ls", "-l", (char *)NULL);
        } else if (strcmp(argv[1], "execle") == 0) {
            char *custom_env[] = { "MY_VAR=Hello from execle", NULL };
            execle("/bin/ls", "ls", "-l", (char *)NULL, custom_env);
        } else if (strcmp(argv[1], "execlp") == 0) {
            execlp("ls", "ls", "-l", (char *)NULL);
        } else if (strcmp(argv[1], "execv") == 0) {
            char *args[] = { "ls", "-l", NULL };
            execv("/bin/ls", args);
        } else if (strcmp(argv[1], "execvp") == 0) {
            char *args[] = { "ls", "-l", NULL };
            execvp("ls", args);
        } else if (strcmp(argv[1], "execvpe") == 0) {
            char *args[] = { "ls", "-l", NULL };
            char *custom_env[] = { "MY_VAR=Hello from execvpe", NULL };
            execvpe("ls", args, custom_env);
        } else {
            fprintf(stderr, "Unknown variant: %s\n", argv[1]);
            exit(EXIT_FAILURE);
        }

        // If any exec call returns, it means it failed.
        fprintf(stderr, "exec failed\n");
        exit(EXIT_FAILURE);

    } else { // Parent Process
        wait(NULL); // Wait for the child to complete
        printf("Parent: Child process has finished.\n");
    }

    return 0;
}