/**
* priority.c
*
* Priority scheduling algorithm.
*
* This scheduler selects the task with the highest priority.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "schedulers.h"
#include "cpu.h"

// The head of the task list
struct node *task_list_head = NULL;

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

    insert(&task_list_head, new_task);
}

/**
 * pickNextTask()
 *
 * Finds and returns the task with the highest priority.
 */
Task *pickNextTask() {
    if (task_list_head == NULL) {
        return NULL;
    }

    struct node *temp = task_list_head;
    Task *highest_priority_task = temp->task;

    while (temp != NULL) {
        if (temp->task->priority > highest_priority_task->priority) {
            highest_priority_task = temp->task;
        }
        temp = temp->next;
    }

    return highest_priority_task;
}

/**
 * schedule()
 *
 * Executes the Priority scheduling algorithm.
 */
void schedule() {
    int current_time = 0;
    int total_wait_time = 0;
    int total_turnaround_time = 0;
    int total_response_time = 0;
    int task_count = 0;

    printf("--- Priority Scheduling ---\n");

    // Count initial tasks
    struct node *counter = task_list_head;
    while(counter != NULL) {
        task_count++;
        counter = counter->next;
    }

    while (task_list_head != NULL) {
        Task *task = pickNextTask();
        run(task, task->burst);

        total_response_time += current_time;
        total_wait_time += current_time;
        current_time += task->burst;
        total_turnaround_time += current_time;

        delete(&task_list_head, task);
    }

    printf("\n--- Priority Performance Metrics ---\n");
    printf("Average Turnaround Time: %.2f\n", (float)total_turnaround_time / task_count);
    printf("Average Response Time: %.2f\n", (float)total_response_time / task_count);
    printf("Average Waiting Time: %.2f\n", (float)total_wait_time / task_count);
}