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

// Initialize a lock for synchronization
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void put(int producer_request_id) {
    buffer[input_index] = producer_request_id;
    printf("Producer: Added request %d to index %d.\n", producer_request_id, input_index);
    input_index = (input_index + 1) % QUEUE_SIZE;
    count++;
}

int get() {
    int consumer_request_id = buffer[output_index];
    printf("Consumer: Processed request %d from index %d.\n", consumer_request_id, output_index);
    output_index = (output_index + 1) % QUEUE_SIZE;
    count--;
    return consumer_request_id;
}

// The producer thread function. Simulates receiving HTTP requests.
void *producer(void *arg) {
    int producer_request_id = 0;

    for (int i = 0; i < NUM_REQUESTS; i++) {
        producer_request_id = i + 1; // Simulate a new request

        // "Busy-wait" if the buffer is full
        // Adds to the time complexity
        while (1) {
            pthread_mutex_lock(&lock);

            if (count < QUEUE_SIZE) {
                // Add request to buffer
                put(producer_request_id);
                pthread_mutex_unlock(&lock);
                break; // Exit busy-wait loop; without break a deadlock occurs
            }

            // If buffer is full, unlock and try again shortly
            pthread_mutex_unlock(&lock);
        }

         // Simulate time to add a request
        sleep(1);
    }
    return NULL;
}

// The consumer thread function. Simulates processing HTTP requests.
void *consumer(void *arg) {
    for (int i = 0; i < NUM_REQUESTS; i++) {

        // Busy-wait if the buffer is empty
        while (1) {
            pthread_mutex_lock(&lock);

            if (count > 0) {
                // Remove request from buffer
                get();
                pthread_mutex_unlock(&lock);
                break;
            }
            
            // If buffer is empty, unlock and try again shortly
            pthread_mutex_unlock(&lock);
        }

        // Simulate time to process a request
        sleep(3);
        // sleep(3) makes it so that the server receives requests fast but is slow at processing them
    }
    return NULL;
}

int main() {
    pthread_t producer_thread, consumer_thread;

    printf("> STARTING SIMULATION USING LOCKS/MUTEX <\n");

    // Create producer and consumer threads
    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    // Wait for threads to finish
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    printf("> SIMULATION COMPLETED <\n");

    return 0;
}