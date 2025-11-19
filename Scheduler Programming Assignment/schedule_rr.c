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
struct node *task_list_head = NULL;

// We need to keep track of original burst times for metric calculations
struct node *original_tasks_head = NULL;

/**
 * add()
 *
 * Adds a task to the list. We also keep a copy of the original
 * task details for calculating metrics later.
 */
void add(char *name, int priority, int burst) {
    // Create the main task to be scheduled
    Task *new_task = malloc(sizeof(Task));
    if (!new_task) {
        fprintf(stderr, "malloc failed in add()\n");
        exit(EXIT_FAILURE);
    }
    new_task->name = strdup(name);
    new_task->initial_burst = burst; // Store original burst time
    new_task->has_been_run = 0;      // Flag to track first run for response time
    new_task->priority = priority;
    new_task->burst = burst;

    // Keep a copy for calculating total burst time for metrics
    Task *original_task = malloc(sizeof(Task));
    original_task->name = strdup(name);
    original_task->burst = burst;
    insert(&original_tasks_head, original_task);

    // Insert into the main task list
    insert(&task_list_head, new_task);
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

    // The list is built by inserting at the head, so it's in reverse order
    // of the input file. We reverse it to get the correct FCFS order for RR.
    struct node *current = task_list_head;
    struct node *prev = NULL, *next = NULL;
    while (current != NULL) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
        task_count++; // Count tasks while we're at it
    }
    task_list_head = prev;

    // Calculate total burst time from the unmodified original list
    struct node *temp_orig = original_tasks_head;
    while (temp_orig != NULL) {
        total_burst_time += temp_orig->task->burst;
        temp_orig = temp_orig->next;
    }

    printf("--- Round-Robin Scheduling (Quantum = %d) ---\n", QUANTUM);
    
    struct node *temp = task_list_head;

    while (task_list_head != NULL) {
        // If we've iterated through the whole list, loop back to the start
        if (temp == NULL) {
            temp = task_list_head;
            // If after resetting, it's still NULL, it means the list is empty.
            if (temp == NULL) break;
        }

        Task *task = temp->task;

        // If task is running for the first time, record response time
        if (task->has_been_run == 0) {
            total_response_time += current_time;
            task->has_been_run = 1;
        }

        // Determine the time slice for this run
        int slice = (task->burst > QUANTUM) ? QUANTUM : task->burst;
        run(task, slice);

        // Update task burst and current time
        task->burst -= slice;
        current_time += slice;

        if (task->burst <= 0) {
            // Task is finished, update turnaround time
            total_turnaround_time += current_time;

            // Delete the task from the list.
            // We must advance our 'temp' pointer *before* deleting the node it points to.
            struct node *to_delete = temp;
            temp = temp->next;
            delete(&task_list_head, to_delete->task);
            // Note: The 'delete' function in list.c doesn't free the task's memory,
            // which is a memory leak. A complete solution would fix that.
        } else {
            // Task is not finished, move to the next one in the list
            temp = temp->next;
        }
    }

    // The total waiting time is the total time all tasks spent in the system
    // minus the time they spent actually running on the CPU.
    total_wait_time = total_turnaround_time - total_burst_time;

    printf("\n--- RR Performance Metrics ---\n");
    if (task_count == 0) return;
    printf("Average Turnaround Time: %.2f\n", (float)total_turnaround_time / task_count);
    printf("Average Response Time: %.2f\n", (float)total_response_time / task_count);
    printf("Average Waiting Time: %.2f\n", (float)total_wait_time / task_count);
}