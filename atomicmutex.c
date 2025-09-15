// lockfree_vs_mutex_counter.c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>

#define THREADS 8
#define OPS_PER_THREAD 10000000

// Lock-free counter (atomic)
atomic_long atomic_counter = 0;

// Mutex-based counter
long mutex_counter = 0;
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

// Time utility (ns)
long long nanos() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long) ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

void* inc_atomic(void* arg) {
    for (long i = 0; i < OPS_PER_THREAD; i++) {
        atomic_fetch_add(&atomic_counter, 1);
    }
    return NULL;
}

void* inc_mutex(void* arg) {
    for (long i = 0; i < OPS_PER_THREAD; i++) {
        pthread_mutex_lock(&counter_mutex);
        mutex_counter++;
        pthread_mutex_unlock(&counter_mutex);
    }
    return NULL;
}

int main() {
    pthread_t threads[THREADS];

    // Benchmark lock-free atomic counter
    long long start = nanos();
    for (int i = 0; i < THREADS; i++)
        pthread_create(&threads[i], NULL, inc_atomic, NULL);

    for (int i = 0; i < THREADS; i++)
        pthread_join(threads[i], NULL);

    long long end = nanos();
    printf("Atomic counter result = %ld, time = %.3f sec\n",
           atomic_counter, (end - start) / 1e9);

    // Benchmark mutex-based counter
    start = nanos();
    for (int i = 0; i < THREADS; i++)
        pthread_create(&threads[i], NULL, inc_mutex, NULL);

    for (int i = 0; i < THREADS; i++)
        pthread_join(threads[i], NULL);

    end = nanos();
    printf("Mutex counter result  = %ld, time = %.3f sec\n",
           mutex_counter, (end - start) / 1e9);

    return 0;
}

