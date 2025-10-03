/**
* rr.c
*
* Round-Robin scheduling algorithm.
*
* Each task is run for a time quantum (or the remainder of its burst).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "schedulers.h"
#include "cpu.h"

// The head of the task list
struct node *g_head = NULL;

// We need to keep track of original burst times for metric calculations
struct node *g_original_tasks = NULL;

/**
 * add()
 *
 * Adds a task to the list. We also keep a copy of the original
 * task details for calculating metrics later.
 */
void add(char *name, int priority, int burst) {
    Task *new_task = malloc(sizeof(Task));
    if (!new_task) {
        fprintf(stderr, "malloc failed in add()\n");
        exit(EXIT_FAILURE);
    }
    new_task->name = strdup(name);
    new_task->priority = priority;
    new_task->burst = burst;
    insert(&g_head, new_task);

    // Keep a copy for metrics
    Task *original_task = malloc(sizeof(Task));
    original_task->name = strdup(name);
    original_task->priority = priority;
    original_task->burst = burst;
    insert(&g_original_tasks, original_task);
}

/**
 * schedule()
 *
 * Executes the Round-Robin scheduling algorithm.
 */
void schedule() {
    int current_time = 0;
    int total_wait_time = 0;
    int total_turnaround_time = 0;
    int total_response_time = 0;
    int task_count = 0;
    int total_burst_time = 0;

    // Reverse the list to get FCFS order for the first round
    struct node *current = g_head;
    struct node *prev = NULL, *next = NULL;
    while (current != NULL) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    g_head = prev;

    // Also reverse the original tasks list and gather metrics
    current = g_original_tasks;
    prev = NULL;
    next = NULL;
    while (current != NULL) {
        task_count++;
        total_burst_time += current->task->burst;

        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    g_original_tasks = prev;

    printf("--- Round-Robin Scheduling (Quantum = %d) ---\n", QUANTUM);
    
    struct node *temp = g_head;

    while (g_head != NULL) {
        // If we've reached the end of the list, loop back to the start
        if (temp == NULL) {
            temp = g_head;
        }

        Task *task = temp->task;

        // Check if this is the first time this task is being run for response time
        struct node *orig_node = g_original_tasks;
        int original_burst = 0;
        while(orig_node) {
            if (strcmp(orig_node->task->name, task->name) == 0) {
                original_burst = orig_node->task->burst;
                break;
            }
            orig_node = orig_node->next;
        }

        if (task->burst == original_burst) {
            total_response_time += current_time;
        }

        // Determine the slice of time to run the task
        int slice = (task->burst > QUANTUM) ? QUANTUM : task->burst;
        run(task, slice);

        task->burst -= slice;
        current_time += slice;

        if (task->burst <= 0) {
            // Task is finished
            total_turnaround_time += current_time;

            // Delete the task and be careful about advancing the pointer
            struct node *to_delete = temp;
            temp = temp->next;
            delete(&g_head, to_delete->task);
        } else {
            // Task is not finished, move to the next one in the list
            temp = temp->next;
        }

    }

    // The total waiting time is the total time all tasks spent in the system
    // minus the time they spent actually running on the CPU.
    total_wait_time = total_turnaround_time - total_burst_time;

    printf("\n--- RR Performance Metrics ---\n");
    printf("Average Turnaround Time: %.2f\n", (float)total_turnaround_time / task_count);
    printf("Average Response Time: %.2f\n", (float)total_response_time / task_count);
    printf("Average Waiting Time: %.2f\n", (float)total_wait_time / task_count);
}