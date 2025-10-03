/**
* priority_rr.c
*
* Priority with Round-Robin scheduling algorithm.
*
* Schedules tasks based on priority, using RR for tasks with equal priority.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "schedulers.h"
#include "cpu.h"

// The head of the task list
struct node *g_head = NULL;

// We need a separate list to keep track of original task details for metrics
struct node *g_original_tasks = NULL;


/**
 * add()
 *
 * Adds a task to the list.
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

    // Keep a copy for metrics. This list will not be modified.
    Task *original_task = malloc(sizeof(Task));
    original_task->name = strdup(name);
    original_task->priority = priority;
    original_task->burst = burst;
    insert(&g_original_tasks, original_task);
}

/**
 * schedule()
 *
 * Executes the Priority with Round-Robin scheduling algorithm.
 */
void schedule() {
    int current_time = 0;
    int total_wait_time = 0;
    int total_turnaround_time = 0;
    int total_response_time = 0;
    int task_count = 0;
    int total_burst_time = 0;

    printf("--- Priority with Round-Robin Scheduling (Quantum = %d) ---\n", QUANTUM);

    // Reverse the original tasks list to easily find original burst times
    // and to count tasks. The order of this list doesn't matter, but reversing
    // makes it consistent with the initial order of g_head if we were to reverse that.
    struct node *temp_orig = g_original_tasks;
    struct node *prev = NULL, *next = NULL;
    while (temp_orig != NULL) {
        task_count++;
        total_burst_time += temp_orig->task->burst;
        next = temp_orig->next;
        temp_orig->next = prev;
        prev = temp_orig;
        temp_orig = next;
    }
    g_original_tasks = prev;


    while (g_head != NULL) {
        // 1. Find the highest priority level among remaining tasks
        int highest_priority = -1;
        struct node *temp = g_head;
        while (temp != NULL) {
            if (temp->task->priority > highest_priority) {
                highest_priority = temp->task->priority;
            }
            temp = temp->next;
        }

        // 3. Run RR on the tasks of the highest priority
        int tasks_at_priority_level = 1; // sentinel to start the loop
        while (tasks_at_priority_level > 0) {
            tasks_at_priority_level = 0;
            temp = g_head; // Start scan from the beginning of the main list
            while (temp != NULL) {
                if (temp->task->priority == highest_priority) {
                    tasks_at_priority_level++;

                    // Check if this is the first time this task is being run
                    // We can check this by comparing its current burst to its original burst
                    struct node *orig_node = g_original_tasks;
                    int original_burst = 0;
                    while(orig_node) {
                        if (strcmp(orig_node->task->name, temp->task->name) == 0) {
                            original_burst = orig_node->task->burst;
                            break;
                        }
                        orig_node = orig_node->next;
                    }

                    if (temp->task->burst == original_burst) {
                        total_response_time += current_time;
                    }

                    int slice = (temp->task->burst > QUANTUM) ? QUANTUM : temp->task->burst;
                    run(temp->task, slice);

                    temp->task->burst -= slice;
                    current_time += slice;


                    if (temp->task->burst <= 0) {
                        // Task finished, remove it from the main list
                        total_turnaround_time += current_time;
                        struct node *to_delete = temp;
                        temp = temp->next; // Move to next before deleting
                        delete(&g_head, to_delete->task);
                        continue; // Continue loop without advancing temp again
                    }
                }
                temp = temp->next;
            }
        }
    }

    // Wait Time = Turnaround Time - Burst Time
    total_wait_time = total_turnaround_time - total_burst_time;

    printf("\n--- Priority RR Performance Metrics ---\n");
    printf("Average Turnaround Time: %.2f\n", (float)total_turnaround_time / task_count);
    printf("Average Response Time: %.2f\n", (float)total_response_time / task_count);
    printf("Average Waiting Time: %.2f\n", (float)total_wait_time / task_count);
}