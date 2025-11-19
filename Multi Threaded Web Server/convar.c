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
int next_request_id = 1;    // Shared counter for unique request IDs
int requests_consumed = 0;  // Shared counter for consumed requests

// Signals
pthread_mutex_t buffer_mutex;
pthread_cond_t buffer_not_full; // Signaled when the buffer has space
pthread_cond_t buffer_not_empty; // Signaled when the buffer has data

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
        pthread_mutex_lock(&buffer_mutex);

        if (next_request_id > NUM_REQUESTS) {
            pthread_mutex_unlock(&buffer_mutex);
            break; // All requests produced, exit loop
        }
        // Wait until there is space in the buffer
        while (count == QUEUE_SIZE) {
            printf("Producer: Waiting because buffer is full...\n");
            pthread_cond_wait(&buffer_not_full, &buffer_mutex);

            // After waking up, re-check if another producer has already finished the work.
            if (next_request_id > NUM_REQUESTS) {
                pthread_mutex_unlock(&buffer_mutex);
                return NULL;
            }
        }

        // Add request to buffer
        int producer_request_id = next_request_id++;
        put(producer_request_id, producer_id);

        // Signal to a consumer that the buffer is no longer empty
        pthread_cond_signal(&buffer_not_empty);
        pthread_mutex_unlock(&buffer_mutex);

        sleep(1); // Simulate time between requests
    }
    return NULL;
}

// The consumer thread function. Simulates processing HTTP requests.
void *consumer(void *arg) {
    // Pass void pointer (consumer ID), cast it to integer, and dereference it
    int consumer_id = *(int*)arg;

    while (1) {
        pthread_mutex_lock(&buffer_mutex);

        if (requests_consumed >= NUM_REQUESTS) {
            pthread_mutex_unlock(&buffer_mutex);
            break; // All requests consumed, exit loop
        }

        // Wait until there is an item in the buffer
        while (count == 0) {
            printf("Consumer: Waiting because buffer is empty...\n");
            pthread_cond_wait(&buffer_not_empty, &buffer_mutex);

            // After waking up, re-check if all work is done
            if (requests_consumed >= NUM_REQUESTS) {
                pthread_mutex_unlock(&buffer_mutex);
                return NULL;
            }
        }

        // Remove request from buffer
        get(consumer_id);
        requests_consumed++;
        
        // Signal to a waiting producer that the buffer is no longer full
        pthread_cond_signal(&buffer_not_full);
        pthread_mutex_unlock(&buffer_mutex);
        sleep(3); // Simulate processing time
    }
    return NULL;
}

int main() {
    int producer_ids[2] = {1, 2};
    int consumer_ids[2] = {1, 2};
    pthread_t producer_thread1, producer_thread2, consumer_thread1, consumer_thread2;

    // Initialize mutex and condition variables
    pthread_mutex_init(&buffer_mutex, NULL);
    pthread_cond_init(&buffer_not_full, NULL);
    pthread_cond_init(&buffer_not_empty, NULL);

    printf("> STARTING SIMULATION USING CONDITIONAL VARIABLES <\n");

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