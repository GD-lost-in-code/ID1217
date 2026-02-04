#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>

#define MAXSIZE 10000
#define MAXTHREADS 10
#define SERIAL_THRESHOLD 1000

/* global array to sort */
int *array;
int n;

/* limit total thread creation */
int maxThreads = MAXTHREADS;
int activeThreads = 1;
pthread_mutex_t threadLock;

/* timer (reused from matrixSum.c style) */
double read_timer() {
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;

    if (!initialized) {
        gettimeofday(&start, NULL);
        initialized = true;
    }
    gettimeofday(&end, NULL);

    return (end.tv_sec - start.tv_sec) +
           1.0e-6 * (end.tv_usec - start.tv_usec);
}

/* argument struct for recursive quicksort */
typedef struct {
    int left;
    int right;
} QSArgs;

/* swap helper */
void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

/* partition function */
int partition(int left, int right) {
    int pivot = array[right];
    int i = left - 1;

    for (int j = left; j < right; j++) {
        if (array[j] <= pivot) {
            i++;
            swap(&array[i], &array[j]);
        }
    }
    swap(&array[i + 1], &array[right]);
    return i + 1;
}

/* sequential quicksort (fallback) */
void quicksort_serial(int left, int right) {
    if (left >= right) return;
    int p = partition(left, right);
    quicksort_serial(left, p - 1);
    quicksort_serial(p + 1, right);
}

/* parallel quicksort worker */
void *parallel_quicksort(void *arg) {
    QSArgs *args = (QSArgs *)arg;
    int left = args->left;
    int right = args->right;

    if (left >= right) return NULL;

    /* small tasks handled serially */
    if (right - left < SERIAL_THRESHOLD) {
        quicksort_serial(left, right);
        return NULL;
    }

    int pivot = partition(left, right);

    QSArgs leftArgs = { left, pivot - 1 };
    QSArgs rightArgs = { pivot + 1, right };

    pthread_t thread;
    bool spawned = false;

    pthread_mutex_lock(&threadLock);
    if (activeThreads < maxThreads) {
        activeThreads++;
        spawned = true;
    }
    pthread_mutex_unlock(&threadLock);

    if (spawned) {
        pthread_create(&thread, NULL, parallel_quicksort, &leftArgs);
        parallel_quicksort(&rightArgs);
        pthread_join(thread, NULL);

        pthread_mutex_lock(&threadLock);
        activeThreads--;
        pthread_mutex_unlock(&threadLock);
    } else {
        quicksort_serial(left, pivot - 1);
        quicksort_serial(pivot + 1, right);
    }

    return NULL;
}

/* main */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: %s n [numThreads]\n", argv[0]);
        return 1;
    }

    n = atoi(argv[1]);
    if (n > MAXSIZE) n = MAXSIZE;

    if (argc > 2)
        maxThreads = atoi(argv[2]);

    array = malloc(n * sizeof(int));
    if (!array) {
        perror("malloc");
        return 1;
    }

    srand(time(NULL));
    for (int i = 0; i < n; i++)
        array[i] = rand() % 100000;

    pthread_mutex_init(&threadLock, NULL);

    double start_time = read_timer();

    QSArgs args = { 0, n - 1 };
    parallel_quicksort(&args);

    double end_time = read_timer();

#ifdef DEBUG
    for (int i = 0; i < n; i++)
        printf("%d ", array[i]);
    printf("\n");
#endif

    printf("Execution time: %g sec\n", end_time - start_time);

    pthread_mutex_destroy(&threadLock);
    free(array);
    return 0;
}
