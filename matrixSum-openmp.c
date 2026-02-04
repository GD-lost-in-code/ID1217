/* matrix summation using OpenMP

   usage with gcc (version 4.2 or higher required):
     gcc -O -fopenmp -o matrixSum-openmp matrixSum-openmp.c 
     ./matrixSum-openmp size numWorkers

*/
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXSIZE 10000
#define MAXWORKERS 8

double start_time, end_time;

int numWorkers;
int size;
int matrix[MAXSIZE][MAXSIZE];

int main(int argc, char *argv[]) {
  int i, j;
  int total = 0;

  int max_val, min_val;
  int max_row = 0, max_col = 0;
  int min_row = 0, min_col = 0;

  /* read command line args */
  size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
  numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
  if (size > MAXSIZE) size = MAXSIZE;
  if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;

  omp_set_num_threads(numWorkers);

  /* initialize the matrix */
  for (i = 0; i < size; i++)
    for (j = 0; j < size; j++)
      matrix[i][j] = rand() % 99;

  /* initialize min and max */
  max_val = matrix[0][0];
  min_val = matrix[0][0];

  start_time = omp_get_wtime();

#pragma omp parallel for reduction(+:total) private(j)
  for (i = 0; i < size; i++) {
    for (j = 0; j < size; j++) {

      int value = matrix[i][j];
      total += value;

      #pragma omp critical
      {
        if (value > max_val) {
          max_val = value;
          max_row = i;
          max_col = j;
        }
        if (value < min_val) {
          min_val = value;
          min_row = i;
          min_col = j;
        }
      }
    }
  }

  end_time = omp_get_wtime();

  printf("Total sum: %d\n", total);
  printf("Maximum value: %d at (%d, %d)\n", max_val, max_row, max_col);
  printf("Minimum value: %d at (%d, %d)\n", min_val, min_row, min_col);
  printf("Execution time: %g seconds\n", end_time - start_time);

  return 0;
}
