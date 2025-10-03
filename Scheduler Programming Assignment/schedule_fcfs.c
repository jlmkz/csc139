/**
* fcfs.c
*
* First-Come, First-Served scheduling algorithm.
*
* This scheduler selects tasks in the order they were added to the list.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "schedulers.h"
#include "cpu.h"

// The head of the task list
struct node *g_head = NULL;

/**
 * add()
 *
 * Adds a task to the list. For FCFS, we can simply insert at the
 * beginning and reverse the list later, or insert at the end.
 * The provided list.c `insert` function adds to the head, so our
 * list will be in reverse order of the input file. We'll correct
 * this in the `schedule` function.
 */
void add(char *name, int priority, int burst) {
    Task *new_task = malloc(sizeof(Task));
    if (!new_task) {
        fprintf(stderr, "malloc failed in add()\n");
        exit(EXIT_FAILURE);
    }

    // strdup allocates memory for the name
    new_task->name = strdup(name);
    new_task->priority = priority;
    new_task->burst = burst;

    // Insert the new task into the list
    insert(&g_head, new_task);
}

/**
 * schedule()
 *
 * Executes the FCFS scheduling algorithm. It processes tasks in the
 * order they appear in the input file. Since our `insert` function
 * adds to the head of the list, the list is currently in reverse order.
 * We will traverse the list from tail to head to simulate FCFS.
 * A simple way to do this is to reverse the list first.
 */
void schedule() {
    // Reverse the list to get the correct FCFS order
    struct node *current = g_head;
    struct node *prev = NULL, *next = NULL;
    while (current != NULL) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    g_head = prev;

    // Now traverse the list in FCFS order
    struct node *temp = g_head;
    int current_time = 0;
    int total_wait_time = 0;
    int total_turnaround_time = 0;
    int total_response_time = 0;
    int task_count = 0;

    printf("--- FCFS Scheduling ---\n");
    while (temp != NULL) {
        run(temp->task, temp->task->burst);
        
        total_response_time += current_time;
        total_wait_time += current_time;
        current_time += temp->task->burst;
        total_turnaround_time += current_time;
        task_count++;
        
        temp = temp->next;
    }

    printf("\n--- FCFS Performance Metrics ---\n");
    // Since all tasks arrive at time 0, response time is the same as waiting time.
    printf("Average Turnaround Time: %.2f\n", (float)total_turnaround_time / task_count);
    printf("Average Response Time: %.2f\n", (float)total_response_time / task_count);
    printf("Average Waiting Time: %.2f\n", (float)total_wait_time / task_count);
}