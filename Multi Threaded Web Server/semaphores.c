// CSC 139 - Multi-threaded Web Server Simulation - Semaphores Approach
// By: James McKenzie
// Credit: Lecture 10-28

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define QUEUE_SIZE 5
#define NUM_REQUESTS 10

// Shared buffer
int buffer[QUEUE_SIZE];
int input_index = 0;
int output_index = 0;
int next_request_id = 1;    // Shared counter for unique request IDs
int requests_consumed = 0;  // Shared counter for consumed requests

// Semaphores
sem_t mutex;       // For mutual exclusion to the buffer
sem_t empty_slots; // Counts empty buffer slots
sem_t full_slots;  // Counts full buffer slots

void put(int producer_request_id, int producer_id) {
    buffer[input_index] = producer_request_id;
    printf("Producer %d: Added request %d to index %d.\n", producer_id, producer_request_id, input_index);
    input_index = (input_index + 1) % QUEUE_SIZE;
    // No need to increment a 'count' variable; semaphores handle it.
}

int get(int consumer_id) {
    int consumer_request_id = buffer[output_index];
    printf("Consumer %d: Processed request %d from index %d.\n", consumer_id, consumer_request_id, output_index);
    output_index = (output_index + 1) % QUEUE_SIZE;
    // No need to decrement a 'count' variable; semaphores handle it.
    return consumer_request_id;
}

// Producer thread. Puts requests into the buffer.
void *producer(void *arg) {
    int producer_id = *(int*)arg;

    while (1) {
        // Wait for an empty slot before trying to produce.
        sem_wait(&empty_slots);

        // Lock to check/update shared state.
        sem_wait(&mutex);

        if (next_request_id > NUM_REQUESTS) {
            // All requests have been generated. Unlock and exit.
            sem_post(&mutex);
            // We must post to empty_slots to unblock any other producers that might be waiting.
            sem_post(&empty_slots);
            break;
        }

        // Produce a new request.
        put(next_request_id, producer_id); // Use the current ID
        next_request_id++;                 // Then increment for the next producer
        sem_post(&mutex);

        // Signal that a slot is now full.
        sem_post(&full_slots);

        sleep(1); // Simulate time between requests
    }
    return NULL;
}

// Consumer thread. Takes requests from the buffer.
void *consumer(void *arg) {
    int consumer_id = *(int*)arg;

    while (1) {
        // Lock to check/update shared state.
        sem_wait(&mutex);

        if (requests_consumed >= NUM_REQUESTS) {
            // All requests have been consumed. Unlock and exit.
            sem_post(&mutex);
            // Post to full_slots to wake up any other consumer that might be
            // waiting on sem_wait(&full_slots). This allows them to re-check the
            // exit condition and terminate as well.
            sem_post(&full_slots);
            break;
        }

        sem_post(&mutex); // Temporarily release mutex before waiting
        // Wait for a full slot before trying to consume.
        sem_wait(&full_slots);
        sem_wait(&mutex); // Re-acquire mutex

        // Consume a request.
        get(consumer_id);
        requests_consumed++;

        sem_post(&mutex);

        // Signal that a slot is now empty.
        sem_post(&empty_slots);

        sleep(3); // Simulate processing time
    }
    return NULL;
}

int main() {
    int producer_ids[2] = {1, 2};
    int consumer_ids[2] = {1, 2};
    pthread_t producer_thread1, producer_thread2, consumer_thread1, consumer_thread2;

    // Initialize semaphores
    sem_init(&mutex, 0, 1);                  // Mutex semaphore, initial value 1
    sem_init(&empty_slots, 0, QUEUE_SIZE);  // Empty slots, initial value QUEUE_SIZE
    sem_init(&full_slots, 0, 0);             // Full slots, initial value 0

    printf("> STARTING SIMULATION USING SEMAPHORES <\n");

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