/* matrix summation using pthreads

   features: uses a barrier; the Worker[0] computes
             the total sum from partial sums computed by Workers
             and prints the total sum to the standard output

   usage under Linux:
     gcc matrixSum.c -lpthread
     a.out size numWorkers

*/
#ifndef _REENTRANT 
#define _REENTRANT 
#endif 
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 10   /* maximum number of workers */

pthread_mutex_t barrier;  /* mutex lock for the barrier */
pthread_cond_t go;        /* condition variable for leaving */
int numWorkers;           /* number of workers */ 
int numArrived = 0;       /* number who have arrived */

/* a reusable counter barrier */
void Barrier() {
  pthread_mutex_lock(&barrier);
  numArrived++;
  if (numArrived == numWorkers) {
    numArrived = 0;
    pthread_cond_broadcast(&go);
  } else
    pthread_cond_wait(&go, &barrier);
  pthread_mutex_unlock(&barrier);
}

/* timer */
double read_timer() {
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if( !initialized )
    {
        gettimeofday( &start, NULL );
        initialized = true;
    }
    gettimeofday( &end, NULL );
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

double start_time, end_time; /* start and end times */
int size, stripSize;  /* assume size is multiple of numWorkers */
typedef struct {
    int sum;
    int max_value, max_row, max_col;
    int min_value, min_row, min_col;
} WorkerResult;

WorkerResult worker_results[MAXWORKERS]; /* partial sums */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */

void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
  int i, j;
  long l; /* use long in case of a 64-bit system */
  pthread_attr_t attr;
  pthread_t workerid[MAXWORKERS];

  /* set global thread attributes */
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  /* initialize mutex and condition variable */
  pthread_mutex_init(&barrier, NULL);
  pthread_cond_init(&go, NULL);

  /* read command line args if any */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
  stripSize = size/numWorkers;

  /* seed RNG
  NOT NECESSARY
  rand isnt entirely random its pseudo-random number generator.
  That means it produces a deterministic sequence of numbers based on a seed(internal valu)
  and when that seed stays the same the result stays the same, as the matrix gets populated 
  by the same "random" numbers. 
  
  srand(time(NULL)); gives a new seed each secnd that passes due to time(NULL)*/

  srand(time(NULL));

  /* initialize the matrix */
  for (i = 0; i < size; i++) {
	  for (j = 0; j < size; j++) {
          matrix[i][j] = rand()%99;
	  }
  }

  /* print the matrix */
#ifdef DEBUG
  for (i = 0; i < size; i++) {
	  printf("[ ");
	  for (j = 0; j < size; j++) {
	    printf(" %d", matrix[i][j]);
	  }
	  printf(" ]\n");
  }
#endif

  /* do the parallel work: create the workers */
  start_time = read_timer();
  for (l = 0; l < numWorkers; l++)
    pthread_create(&workerid[l], &attr, Worker, (void *) l);
  pthread_exit(NULL);
}

/* Each worker sums the values in one strip of the matrix.
   After a barrier, worker(0) computes and prints the total */
void *Worker(void *arg) {
  long myid = (long) arg;
  int value, i, j, first, last;

  int local_sum;

  int local_max, local_min;
  int local_max_row, local_max_col;
  int local_min_row, local_min_col;

#ifdef DEBUG
  printf("worker %d (pthread id %d) has started\n", myid, pthread_self());
#endif

  /* determine first and last rows of my strip */
  first = myid*stripSize;
  last = (myid == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);

  local_sum = 0;
  local_max = matrix[first][0];
  local_min = matrix[first][0];
  local_max_row = first;
  local_max_col = 0;
  local_min_row = first;
  local_min_col = 0;

  /* sum values in my strip */
  for (i = first; i <= last; i++){
    for (j = 0; j < size; j++){
      value = matrix[i][j];

      local_sum += value;
      /* max */
      if (value > local_max) {
          local_max = value;
          local_max_row = i;
          local_max_col = j;
      }

      /* min */
      if (value < local_min) {
          local_min = value;
          local_min_row = i;
          local_min_col = j;
      }
    }
  }

  worker_results[myid].sum = local_sum;

  worker_results[myid].max_value = local_max;
  worker_results[myid].max_row   = local_max_row;
  worker_results[myid].max_col   = local_max_col;

  worker_results[myid].min_value = local_min;
  worker_results[myid].min_row   = local_min_row;
  worker_results[myid].min_col   = local_min_col;

  Barrier();
  
  if (myid == 0) {
    int global_sum = 0;

    int global_max = worker_results[0].max_value;
    int global_max_row = worker_results[0].max_row;
    int global_max_col = worker_results[0].max_col;

    int global_min = worker_results[0].min_value;
    int global_min_row = worker_results[0].min_row;
    int global_min_col = worker_results[0].min_col;

    for (i = 0; i < numWorkers; i++){
      global_sum += worker_results[i].sum;

      // Max
      if (worker_results[i].max_value > global_max) {
          global_max = worker_results[i].max_value;
          global_max_row = worker_results[i].max_row;
          global_max_col = worker_results[i].max_col;
      }

      // Min
      if (worker_results[i].min_value < global_min) {
          global_min = worker_results[i].min_value;
          global_min_row = worker_results[i].min_row;
          global_min_col = worker_results[i].min_col;
      }
    }
    /* get end time */
    end_time = read_timer();
    /* print results */
    printf("The total is %d\n", global_sum);
    printf("Maximum value: %d at (%d,%d)\n", global_max, global_max_row, global_max_col);
    printf("Minimum value: %d at (%d,%d)\n", global_min, global_min_row, global_min_col);
    printf("The execution time is %g sec\n", end_time - start_time);
  }
}
