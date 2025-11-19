// CSC 139 - Multi-threaded Web Server Simulation - Conditional Variables Approach
// By: James McKenzie
// Credit: Lecture 10-23-25

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define QUEUE_SIZE 5
#define NUM_REQUESTS 10

// Shared resource and state variables
int buffer[QUEUE_SIZE];
int count = 0;
int input_index = 0;
int output_index = 0;

// Signals
pthread_mutex_t buffer_mutex;
pthread_cond_t buffer_not_full; // Signaled when the buffer has space
pthread_cond_t buffer_not_empty; // Signaled when the buffer has data

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
    for (int i = 0; i < NUM_REQUESTS; i++) {
        int consumer_request_id = i + 1; // Simulate a new request

        pthread_mutex_lock(&buffer_mutex);

        // Wait until there is space in the buffer
        while (count == QUEUE_SIZE) {
            printf("Producer: Buffer is full. Waiting...\n");
            pthread_cond_wait(&buffer_not_full, &buffer_mutex);
        }

        // Add request to buffer
        put(consumer_request_id);

        // Signal to a consumer that the buffer is no longer empty
        pthread_cond_signal(&buffer_not_empty);
        pthread_mutex_unlock(&buffer_mutex);

        sleep(1); // Simulate time between requests 
    }
    return NULL;
}

// The consumer thread function. Simulates processing HTTP requests.
void *consumer(void *arg) {
    for (int i = 0; i < NUM_REQUESTS; i++) {
        pthread_mutex_lock(&buffer_mutex);

        // Wait until there is an item in the buffer
        while (count == 0) {
            printf("Consumer: Buffer is empty. Waiting...\n");
            pthread_cond_wait(&buffer_not_empty, &buffer_mutex);
        }

        // Remove request from buffer
        get();

        // Signal to a waiting producer that the buffer is no longer full
        pthread_cond_signal(&buffer_not_full);
        pthread_mutex_unlock(&buffer_mutex);
        
        sleep(3); // Simulate processing time
    }
    return NULL;
}

int main() {
    pthread_t producer_thread, consumer_thread;

    // Initialize mutex and condition variables
    pthread_mutex_init(&buffer_mutex, NULL);
    pthread_cond_init(&buffer_not_full, NULL);
    pthread_cond_init(&buffer_not_empty, NULL);

    printf("> STARTING SIMULATION USING CONDITIONAL VARIABLES <\n");

    // Create producer and consumer threads
    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    // Wait for threads to finish
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    printf("> SIMULATION COMPLETED <\n");

    return 0;
}