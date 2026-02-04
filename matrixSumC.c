#ifndef _REENTRANT 
#define _REENTRANT 
#endif 
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>
#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 10   /* maximum number of workers */

int size, numWorkers;
int matrix[MAXSIZE][MAXSIZE];

/* PART C: shared global results */
int global_sum = 0;

int global_max = INT_MIN;
int global_max_row = -1;
int global_max_col = -1;

int global_min = INT_MAX;
int global_min_row = -1;
int global_min_col = -1;

/* PART C: shared "bag-of-tasks" row counter */
int nextRow = 0;

/* PART C: mutex for shared data (global results + nextRow) */
pthread_mutex_t lock;

/* timer */
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

void *Worker(void *arg);

int main(int argc, char *argv[]) {
    int i, j;
    long l;
    pthread_t workerid[MAXWORKERS];
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    pthread_mutex_init(&lock, NULL);

    size = (argc > 1) ? atoi(argv[1]) : MAXSIZE;
    numWorkers = (argc > 2) ? atoi(argv[2]) : MAXWORKERS;

    if (size > MAXSIZE) size = MAXSIZE;
    if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;

    srand(time(NULL));
    for (i = 0; i < size; i++)
        for (j = 0; j < size; j++)
            matrix[i][j] = rand() % 99;

    double start_time = read_timer();

    /* create workers */
    for (l = 0; l < numWorkers; l++)
        pthread_create(&workerid[l], &attr, Worker, (void *)l);

    /* wait for all workers to finish */
    for (i = 0; i < numWorkers; i++)
        pthread_join(workerid[i], NULL);

    double end_time = read_timer();

    /* main thread prints results */
    printf("The total is %d\n", global_sum);
    printf("Maximum value: %d at (%d,%d)\n", 
           global_max, global_max_row, global_max_col);
    printf("Minimum value: %d at (%d,%d)\n", 
           global_min, global_min_row, global_min_col);
    printf("The execution time is %g sec\n", end_time - start_time);

    pthread_mutex_destroy(&lock);
    return 0;
}

void *Worker(void *arg) {
    long myid = (long)arg;
    int row, i, j, value;

    int local_sum;
    int local_max, local_min;
    int local_max_row, local_max_col;
    int local_min_row, local_min_col;

    while (true) {
        /* fetch a row from the "bag" */
        pthread_mutex_lock(&lock);
        row = nextRow++;
        pthread_mutex_unlock(&lock);

        if (row >= size)
            break; /* no more rows */

        /* initialize local variables for this row */
        local_sum = 0;
        local_max = matrix[row][0];
        local_min = matrix[row][0];
        local_max_row = row;
        local_max_col = 0;
        local_min_row = row;
        local_min_col = 0;

        for (j = 0; j < size; j++) {
            value = matrix[row][j];
            local_sum += value;

            if (value > local_max) {
                local_max = value;
                local_max_col = j;
            }

            if (value < local_min) {
                local_min = value;
                local_min_col = j;
            }
        }

        /* update global results safely */
        pthread_mutex_lock(&lock);
        global_sum += local_sum;

        if (local_max > global_max) {
            global_max = local_max;
            global_max_row = local_max_row;
            global_max_col = local_max_col;
        }

        if (local_min < global_min) {
            global_min = local_min;
            global_min_row = local_min_row;
            global_min_col = local_min_col;
        }
        pthread_mutex_unlock(&lock);
    }

    return NULL;
}
