/**
* sjf.c
*
* Shortest-Job-First scheduling algorithm.
*
* This scheduler selects the task with the shortest CPU burst time.
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
 * Finds and returns the task with the shortest burst time.
 */
Task *pickNextTask() {
    if (task_list_head == NULL) {
        return NULL;
    }

    struct node *temp = task_list_head;
    Task *shortest_job = temp->task;

    while (temp != NULL) {
        if (temp->task->burst < shortest_job->burst) {
            shortest_job = temp->task;
        }
        temp = temp->next;
    }

    return shortest_job;
}

/**
 * schedule()
 *
 * Executes the SJF scheduling algorithm.
 */
void schedule() {
    int current_time = 0;
    int total_wait_time = 0;
    int total_turnaround_time = 0;
    int total_response_time = 0;
    int task_count = 0;

    printf("--- SJF Scheduling ---\n");

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

    printf("\n--- SJF Performance Metrics ---\n");
    printf("Average Turnaround Time: %.2f\n", (float)total_turnaround_time / task_count);
    printf("Average Response Time: %.2f\n", (float)total_response_time / task_count);
    printf("Average Waiting Time: %.2f\n", (float)total_wait_time / task_count);
}