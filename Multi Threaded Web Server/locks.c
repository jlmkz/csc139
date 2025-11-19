// CSC 139 - Multi-threaded Web Server Simulation - Locks/Mutex Approach
// By: James McKenzie
// Credit: Lecture 10-23-25

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> // To use pthreads
#include <unistd.h>

// Had to declare global variables using #define because variable length array
// declaration is not allowed at file scope (ex: int QUEUE_SIZE = 5 messes with
// line 16)
#define QUEUE_SIZE 5        // Size of the request queue
#define NUM_REQUESTS 10     // Total number of requests to produce

// Shared resource: a fixed-size buffer for requests
int buffer[QUEUE_SIZE];
int count = 0;              // Number of items in the buffer
int input_index = 0;        // Index for producer to write to
int output_index = 0;       // Index for consumer to read from
int next_request_id = 1;    // Shared counter for unique request IDs
int requests_consumed = 0;  // Shared counter for consumed requests

// Initialize a lock for synchronization
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void put(int producer_request_id, int producer_id) {
    buffer[input_index] = producer_request_id;
    printf("Producer %d: Added request %d to index %d.\n", producer_id, producer_request_id, input_index);
    input_index = (input_index + 1) % QUEUE_SIZE;
    count++;
}

int get(int consumer_id) {
    int consumer_request_id = buffer[output_index];
    printf("Consumer %d: Processed request %d from index %d.\n", consumer_id, consumer_request_id, output_index);
    output_index = (output_index + 1) % QUEUE_SIZE;
    count--;
    return consumer_request_id;
}

// The producer thread function. Simulates receiving HTTP requests.
void *producer(void *arg) {
    // Pass void pointer (producer ID), cast it to integer, and dereference it
    int producer_id = *(int*)arg;

    while (1) {
        pthread_mutex_lock(&lock); // Lock the critical section

        if (next_request_id > NUM_REQUESTS) {
            // All requests have been generated; exit.
            pthread_mutex_unlock(&lock);
            break;
        }

        if (count < QUEUE_SIZE) {
            // There's space, produce a request
            int producer_request_id = next_request_id++;
            put(producer_request_id, producer_id);
            pthread_mutex_unlock(&lock); // Unlock after producing
            sleep(1); // Simulate time to generate next request
        } else {
            // Buffer is full, unlock and yield to let consumers work.
            pthread_mutex_unlock(&lock);
        }
    }
    return NULL;
}

// The consumer thread function. Simulates processing HTTP requests.
void *consumer(void *arg) {
    // Pass void pointer (producer ID), cast it to integer, and dereference it
    int consumer_id = *(int*)arg;

    while (1) {
        pthread_mutex_lock(&lock); // Lock the critical section

        if (requests_consumed >= NUM_REQUESTS) {
            // All requests have been consumed; exit.
            pthread_mutex_unlock(&lock);
            break;
        }

        if (count > 0) {
            // There's a request, so consume it
            get(consumer_id);
            requests_consumed++;
            pthread_mutex_unlock(&lock); // Unlock after consuming
            sleep(3); // Simulate processing time
            // sleep(3) makes it so that the server receives requests fast but is slow at processing them
        } else {
            // Buffer is empty, but more requests may arrive
            // Unlock so other producers can use it
            pthread_mutex_unlock(&lock);
        }
    }
    return NULL;
}

int main() {
    int producer_ids[2] = {1, 2};
    int consumer_ids[2] = {1, 2};

    pthread_t producer_thread1, producer_thread2, consumer_thread1, consumer_thread2;

    printf("> STARTING SIMULATION USING LOCKS/MUTEX <\n");

    // Create producer and consumer threads
    pthread_create(&producer_thread1, NULL, producer, &producer_ids[0]);
    pthread_create(&producer_thread2, NULL, producer, &producer_ids[1]);
    pthread_create(&consumer_thread1, NULL, consumer, &consumer_ids[0]);
    pthread_create(&consumer_thread2, NULL, consumer, &consumer_ids[1]);

    // Wait for threads to finish
    pthread_join(producer_thread1, NULL);
    pthread_join(producer_thread2, NULL);
    pthread_join(consumer_thread1, NULL);
    pthread_join(consumer_thread2, NULL);

    printf("> SIMULATION COMPLETED <\n");

    return 0;
}