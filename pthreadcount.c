#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>

#define MAX_THREADS 4   // Change based on CPU cores

typedef struct {
    char *data;
    long start;
    long end;
    long count;
} ThreadData;

void* count_words(void* arg) {
    ThreadData* td = (ThreadData*)arg;
    int inWord = 0;
    td->count = 0;

    for (long i = td->start; i < td->end; i++) {
        if (isspace(td->data[i]) || td->data[i] == '\0') {
            if (inWord) {
                td->count++;
                inWord = 0;
            }
        } else {
            inWord = 1;
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // Read file into memory
    FILE* fp = fopen(argv[1], "r");
    if (!fp) {
        perror("File open failed");
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    char* buffer = malloc(size + 1);
    fread(buffer, 1, size, fp);
    buffer[size] = '\0';
    fclose(fp);

    // Setup threads
    pthread_t threads[MAX_THREADS];
    ThreadData td[MAX_THREADS];
    long chunk = size / MAX_THREADS;

    for (int i = 0; i < MAX_THREADS; i++) {
        td[i].data = buffer;
        td[i].start = i * chunk;
        td[i].end = (i == MAX_THREADS - 1) ? size : (i + 1) * chunk;

        // Avoid cutting a word in half
        if (i != MAX_THREADS - 1) {
            while (td[i].end < size && !isspace(buffer[td[i].end]))
                td[i].end++;
        }

        pthread_create(&threads[i], NULL, count_words, &td[i]);
    }

    // Collect results
    long total = 0;
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
        total += td[i].count;
    }

    printf("Total words: %ld\n", total);
    free(buffer);
    return 0;
}

