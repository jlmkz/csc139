// CSC 139 - Context Switch Practice - James McKenzie
#define _GNU_SOURCE // For sched_setaffinity
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sched.h>

/**
 * This program measures the approximate cost of either a system call or a
 * context switch, based on a command-line argument.
 *
 * To measure system call cost:
 *   ./os_measurements syscall
 *
 * To measure context switch cost:
 *   ./os_measurements context
 */

// A helper function to calculate the time difference in microseconds
long long timeval_diff_us(struct timeval *start, struct timeval *end) {
    return (end->tv_sec - start->tv_sec) * 1000000LL + (end->tv_usec - start->tv_usec);
}

/**
 * Measures the cost of a minimal system call by timing a tight loop of
 * 0-byte read() calls.
 */
void measure_syscall() {
    struct timeval start, end;
    long long elapsed_us; // us = microseconds
    int iterations = 1000000; // One million iterations

    // First, get a baseline for timer resolution
    gettimeofday(&start, NULL);
    gettimeofday(&end, NULL);
    printf("Measuring timer precision...\n");
    printf("Time for back-to-back gettimeofday(): %lld microseconds.\n", timeval_diff_us(&start, &end));

    // Now, measure the system call
    printf("Measuring cost of a minimal system call (read(0, NULL, 0))...\n");
    printf("Performing %d iterations...\n", iterations);

    gettimeofday(&start, NULL);

    for (int i = 0; i < iterations; i++) {
        read(0, NULL, 0); // Minimal system call
    }

    gettimeofday(&end, NULL);

    elapsed_us = timeval_diff_us(&start, &end);
    double average_ns = (double)elapsed_us * 1000.0 / iterations;

    printf("Total time for %d iterations: %lld microseconds.\n", iterations, elapsed_us);
    printf("Average time per system call: ~%.2f nanoseconds.\n", average_ns);
}

/**
 * Measures the cost of a context switch using two processes communicating
 * over two pipes, forcing the OS to switch between them.
 */
void measure_context_switch() {
    int pipe1[2]; // Parent writes, Child reads
    int pipe2[2]; // Child writes, Parent reads
    pid_t pid;
    struct timeval start, end;
    int iterations = 100000;

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe");
        exit(1);
    }

    // Pin to a single CPU to ensure we measure a context switch, not core migration.
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set); // Pin to CPU 0

    if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &set) == -1) {
        fprintf(stderr, "Warning: Could not pin parent to CPU 0. Measurement may be less accurate.\n");
    }

    pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    }
    if (pid == 0) { // --- Child Process ---
        if (sched_setaffinity(getpid(), sizeof(cpu_set_t), &set) == -1) {
             fprintf(stderr, "Warning: Could not pin child to CPU 0.\n");
        }

        close(pipe1[1]); // Child doesn't write to pipe1
        close(pipe2[0]); // Child doesn't read from pipe2

        char buf;
        for (int i = 0; i < iterations; i++) {
            read(pipe1[0], &buf, 1);  // Wait for parent, triggers context switch
            write(pipe2[1], &buf, 1); // Signal parent, triggers context switch
        }
        exit(0);
    } else { // --- Parent Process ---
        close(pipe1[0]); // Parent doesn't read from pipe1
        close(pipe2[1]); // Parent doesn't write to pipe2

        printf("Measuring context switch cost...\n");
        printf("Performing %d round trips (2 context switches per trip)...\n", iterations);

        gettimeofday(&start, NULL);

        char buf = 'p';
        for (int i = 0; i < iterations; i++) {
            write(pipe1[1], &buf, 1); // Signal child
            read(pipe2[0], &buf, 1);  // Wait for child
        }

        gettimeofday(&end, NULL);

        wait(NULL); // Wait until child terminates

        long long elapsed_us = timeval_diff_us(&start, &end);
        // Each round trip is 2 context switches (Parent->Child, Child->Parent)
        double time_per_switch_us = (double)elapsed_us / (iterations * 2.0);

        printf("Total time for %d round trips: %lld microseconds.\n", iterations, elapsed_us);
        printf("Average time per context switch: ~%.3f microseconds.\n", time_per_switch_us);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <syscall | context>\n", argv[0]);
        exit(1);
    }

    if (strcmp(argv[1], "syscall") == 0) {
        measure_syscall();
    } else if (strcmp(argv[1], "context") == 0) {
        measure_context_switch();
    } else {
        fprintf(stderr, "Invalid measurement type: '%s'. Please use 'syscall' or 'context'.\n", argv[1]);
        exit(1);
    }

    exit(0);
}