#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAXSIZE 10000
#define SERIAL_THRESHOLD 1000

int *array;

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

/* OpenMP task-based quicksort */
void quicksort_parallel(int left, int right) {
    if (left >= right) return;

    /* small subarrays handled serially */
    if (right - left < SERIAL_THRESHOLD) {
        quicksort_serial(left, right);
        return;
    }

    int pivot = partition(left, right);

    #pragma omp task
    quicksort_parallel(left, pivot - 1);

    #pragma omp task
    quicksort_parallel(pivot + 1, right);

    #pragma omp taskwait
}

/* main */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: %s n [numThreads]\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    if (n > MAXSIZE) n = MAXSIZE;

    int numThreads = (argc > 2) ? atoi(argv[2]) : 4;
    omp_set_num_threads(numThreads);

    array = malloc(n * sizeof(int));
    if (!array) {
        perror("malloc");
        return 1;
    }

    srand(time(NULL));
    for (int i = 0; i < n; i++)
        array[i] = rand() % 100000;

    double start_time = omp_get_wtime();

    /* start parallel region with a single initial task */
    #pragma omp parallel
    {
        #pragma omp single
        quicksort_parallel(0, n - 1);
    }

    double end_time = omp_get_wtime();

    printf("Execution time: %g sec\n", end_time - start_time);

    free(array);
    return 0;
}
