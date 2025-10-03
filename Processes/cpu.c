#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // For sleep()


int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: cpu <string>\n");
        exit(1);
    }
    char *str = argv[1];
    while (1) {
        // The original code had an undefined function 'Spin(1)'.
        // This is likely from the OSTEP textbook examples, where it's a busy-wait loop.
        // We'll replace it with sleep(1) to pause for one second between prints.
        sleep(1);
        printf("%s\n", str);
    }
    return 0;
}