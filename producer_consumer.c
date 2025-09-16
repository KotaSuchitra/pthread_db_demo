#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5      // Queue capacity
#define PRODUCERS 2        // Number of producers
#define CONSUMERS 2        // Number of consumers
#define ITEMS_PER_PRODUCER 5

int buffer[BUFFER_SIZE];
int count = 0, in = 0, out = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full  = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

void *producer(void *arg) {
    int id = *(int *)arg;
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int item = rand() % 100;

        pthread_mutex_lock(&mutex);
        while (count == BUFFER_SIZE) {
            printf("Producer %d waiting (buffer full)\n", id);
            pthread_cond_wait(&not_full, &mutex);
        }

        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        count++;
        printf("Producer %d produced %d (count=%d)\n", id, item, count);

        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);

        sleep(1); // simulate work
    }
    return NULL;
}

void *consumer(void *arg) {
    int id = *(int *)arg;
    for (;;) {
        pthread_mutex_lock(&mutex);
        while (count == 0) {
            printf("Consumer %d waiting (buffer empty)\n", id);
            pthread_cond_wait(&not_empty, &mutex);
        }

        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;
        printf("Consumer %d consumed %d (count=%d)\n", id, item, count);

        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);

        sleep(2); // simulate work
    }
    return NULL;
}

int main() {
    pthread_t prod[PRODUCERS], cons[CONSUMERS];
    int ids[PRODUCERS > CONSUMERS ? PRODUCERS : CONSUMERS];

    // Start consumers
    for (int i = 0; i < CONSUMERS; i++) {
        ids[i] = i + 1;
        pthread_create(&cons[i], NULL, consumer, &ids[i]);
    }

    // Start producers
    for (int i = 0; i < PRODUCERS; i++) {
        ids[i] = i + 1;
        pthread_create(&prod[i], NULL, producer, &ids[i]);
    }

    // Join producers (they finish)
    for (int i = 0; i < PRODUCERS; i++) {
        pthread_join(prod[i], NULL);
    }

    // Consumers run forever in this demo; in real use, you’d signal them to stop
    sleep(10);
    printf("Main exiting...\n");
    return 0;
}
