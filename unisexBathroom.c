#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define NUM_MEN 5
#define NUM_WOMEN 5

/* ---------------- Shared State ---------------- */

int men_count = 0;
int women_count = 0;

/* Semaphores */
sem_t mutex;        // protects counters
sem_t bathroom;     // controls bathroom access
sem_t turnstile;    // fairness control

/* ---------------- Thread Functions ---------------- */

void* man_thread(void* arg)
{
    int id = *((int*)arg);

    while (1)
    {
        /* Simulate working outside bathroom */
        sleep(rand() % 3 + 1);

        printf("Man %d wants to enter\n", id);

        /* ===== ENTRY SECTION ===== */
        sem_wait(&mutex);
            men_count++;
            if (men_count == 1)
                sem_wait(&bathroom);
        sem_post(&mutex);

        printf("Man %d enters bathroom\n", id);
        printf("DEBUG: men_count=%d women_count=%d\n", men_count, women_count);


        /* Simulate using bathroom */
        sleep(rand() % 2 + 1);

        printf("DEBUG: men_count=%d women_count=%d\n", men_count, women_count);
        printf("Man %d leaving bathroom\n", id);

        /* ===== EXIT SECTION ===== */
        sem_wait(&mutex);
            men_count--;
            if (men_count == 0)
                sem_post(&bathroom);
        sem_post(&mutex);
    }

    return NULL;
}

void* woman_thread(void* arg)
{
    int id = *((int*)arg);

    while (1)
    {
        sleep(rand() % 3 + 1);

        printf("Woman %d wants to enter\n", id);

        /* ===== ENTRY SECTION ===== */
        sem_wait(&mutex);
            women_count++;
            if (women_count == 1)
                sem_wait(&bathroom);
        sem_post(&mutex);

        printf("Woman %d enters bathroom\n", id);
        printf("DEBUG: men_count=%d women_count=%d\n", men_count, women_count);

        sleep(rand() % 2 + 1);

        printf("DEBUG: men_count=%d women_count=%d\n", men_count, women_count);
        printf("Woman %d leaving bathroom\n", id);

        /* ===== EXIT SECTION ===== */
        sem_wait(&mutex);
            women_count--;
            if (women_count == 0)
                sem_post(&bathroom);
        sem_post(&mutex);
    }

    return NULL;
}

/* ---------------- Main ---------------- */

int main()
{
    pthread_t men[NUM_MEN];
    pthread_t women[NUM_WOMEN];

    int men_ids[NUM_MEN];
    int women_ids[NUM_WOMEN];

    srand(time(NULL));

    /* Initialize semaphores */
    sem_init(&mutex, 0, 1);       // binary semaphore
    sem_init(&bathroom, 0, 1);    // bathroom initially free
    sem_init(&turnstile, 0, 1);   // fairness gate

    /* Create men threads */
    for (int i = 0; i < NUM_MEN; i++)
    {
        men_ids[i] = i;
        pthread_create(&men[i], NULL, man_thread, &men_ids[i]);
    }

    /* Create women threads */
    for (int i = 0; i < NUM_WOMEN; i++)
    {
        women_ids[i] = i;
        pthread_create(&women[i], NULL, woman_thread, &women_ids[i]);
    }

    /* Join threads (program runs forever) */
    for (int i = 0; i < NUM_MEN; i++)
        pthread_join(men[i], NULL);

    for (int i = 0; i < NUM_WOMEN; i++)
        pthread_join(women[i], NULL);

    return 0;
}
