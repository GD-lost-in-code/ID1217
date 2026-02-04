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

int size, stripSize, numWorkers;
int matrix[MAXSIZE][MAXSIZE];

/* PART B: shared global results */
int global_sum = 0;

int global_max = INT_MIN;
int global_max_row = -1;
int global_max_col = -1;

int global_min = INT_MAX;
int global_min_row = -1;
int global_min_col = -1;

/* PART B: mutex for shared data */
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

void *Worker(void *);

int main(int argc, char *argv[]) {
    int i, j;
    long l;
    pthread_t workerid[MAXWORKERS];
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    pthread_mutex_init(&lock, NULL);   // PART B

    size = (argc > 1) ? atoi(argv[1]) : MAXSIZE;
    numWorkers = (argc > 2) ? atoi(argv[2]) : MAXWORKERS;

    if (size > MAXSIZE) size = MAXSIZE;
    if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;

    stripSize = size / numWorkers;

    /* seed RNG
    NOT NECESSARY
    rand isnt entirely random its pseudo-random number generator.
    That means it produces a deterministic sequence of numbers based on a seed(internal valu)
    and when that seed stays the same the result stays the same, as the matrix gets populated 
    by the same "random" numbers. 
    
    srand(time(NULL)); gives a new seed each secnd that passes due to time(NULL)*/

    srand(time(NULL));
    for (i = 0; i < size; i++)
        for (j = 0; j < size; j++)
            matrix[i][j] = rand() % 99;

    double start_time = read_timer();

    for (l = 0; l < numWorkers; l++)
        pthread_create(&workerid[l], &attr, Worker, (void *)l);

    /* PART B: wait for all workers */
    for (i = 0; i < numWorkers; i++)
        pthread_join(workerid[i], NULL);

    double end_time = read_timer();

    /* PART B: main thread prints */
    printf("The total is %d\n", global_sum);
    printf("Maximum value: %d at (%d,%d)\n",
           global_max, global_max_row, global_max_col);
    printf("Minimum value: %d at (%d,%d)\n",
           global_min, global_min_row, global_min_col);
    printf("The execution time is %g sec\n",
           end_time - start_time);

    pthread_mutex_destroy(&lock);
    return 0;
}

void *Worker(void *arg) {
    long myid = (long)arg;
    int i, j, first, last;
    int value;

    int local_sum = 0;
    int local_max, local_min;
    int local_max_row, local_max_col;
    int local_min_row, local_min_col;

    first = myid * stripSize;
    last = (myid == numWorkers - 1) ? size - 1 : first + stripSize - 1;

    local_max = matrix[first][0];
    local_min = matrix[first][0];
    local_max_row = first;
    local_max_col = 0;
    local_min_row = first;
    local_min_col = 0;

    for (i = first; i <= last; i++) {
        for (j = 0; j < size; j++) {
            value = matrix[i][j];
            local_sum += value;

            if (value > local_max) {
                local_max = value;
                local_max_row = i;
                local_max_col = j;
            }

            if (value < local_min) {
                local_min = value;
                local_min_row = i;
                local_min_col = j;
            }
        }
    }

    /* PART B: single protected update */
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
    return NULL;
}
