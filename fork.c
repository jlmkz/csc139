#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    int x = 100;

    printf("PARENT (pid:%d): value of x before fork is %d\n", (int) getpid(), x);

    pid_t pid = fork();

    if (pid < 0) {
        // fork failed
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (pid == 0) {
        // child process
        printf("CHILD (pid:%d): initial value of x is %d\n", (int) getpid(), x);

        // Child modifies its copy of x
        x = 50;
        printf("CHILD (pid:%d): changed x to %d\n", (int) getpid(), x);

        return 0; // Child exits
    } else {
        // parent process

        // Parent modifies its copy of x
        x = 200;
        printf("PARENT (pid:%d): changed x to %d\n", (int) getpid(), x);

        // Parent waits for the child to complete
        wait(NULL);

        printf("PARENT (pid:%d): value of x after child has finished is %d\n", (int) getpid(), x);
    }

    return 0;
}

/*
1. What value is the variable in the child process? The child process receives a complete copy of the parent's address space. 
This means that any variables in the parent process at the time of the fork() call will have the exact same value in the 
newly created child process. So, if x is 100 in the parent before fork(), it will also be 100 in the child right after fork().

2. What happens to the variable when both the child and parent change the value of x? This is where the concept of separate address spaces
becomes crucial. Although the child inherits the parent's data, it is a copy. After the fork() call, the parent and child have independent 
memory spaces.

If the child process changes its value of x, it is only modifying its own copy. The parent's value of x remains completely unaffected. 
Likewise, if the parent changes its x, the child's copy is not changed. They are operating on two independent variables that just happened 
to start with the same value.
*/

