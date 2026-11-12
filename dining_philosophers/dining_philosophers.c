// Tarea 3: filosofos cenando, solucion libre de deadlock con semaforo sala

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NUM_FILOSOFOS        5
#define COMIDAS_POR_FILOSOFO 3

pthread_mutex_t tenedor[NUM_FILOSOFOS];
sem_t sala; /* a lo sumo NUM_FILOSOFOS-1 filosofos pueden intentar comer */

static int comidas_hechas[NUM_FILOSOFOS];

static void pensar(int id) {
    printf("Filosofo %d esta pensando...\n", id);
    usleep(200000 + rand() % 300000);
}

static void comer(int id) {
    printf("Filosofo %d esta comiendo...\n", id);
    usleep(100000 + rand() % 200000);
}

static void *filosofo(void *arg) {
    long id = (long)arg;
    int izquierdo = (int)id;
    int derecho   = ((int)id + 1) % NUM_FILOSOFOS;

    /* Orden consistente de tenedores para evitar deadlock circular */
    int primero = izquierdo < derecho ? izquierdo : derecho;
    int segundo = izquierdo < derecho ? derecho : izquierdo;

    for (int i = 0; i < COMIDAS_POR_FILOSOFO; i++) {
        pensar((int)id);

        sem_wait(&sala); /* permiso para intentar comer (max N-1) */

        pthread_mutex_lock(&tenedor[primero]);
        pthread_mutex_lock(&tenedor[segundo]);

        comer((int)id);
        comidas_hechas[id]++;

        pthread_mutex_unlock(&tenedor[segundo]);
        pthread_mutex_unlock(&tenedor[primero]);

        sem_post(&sala);
    }

    printf("Filosofo %d termino (%d comidas).\n", (int)id, comidas_hechas[id]);
    return NULL;
}

int main(void) {
    srand((unsigned)time(NULL));

    for (int i = 0; i < NUM_FILOSOFOS; i++)
        pthread_mutex_init(&tenedor[i], NULL);

    /* Sala con capacidad N-1: garantiza que al menos un tenedor quede libre */
    sem_init(&sala, 0, NUM_FILOSOFOS - 1);

    pthread_t hilos[NUM_FILOSOFOS];
    for (long i = 0; i < NUM_FILOSOFOS; i++)
        pthread_create(&hilos[i], NULL, filosofo, (void *)i);

    for (int i = 0; i < NUM_FILOSOFOS; i++)
        pthread_join(hilos[i], NULL);

    int total = 0;
    for (int i = 0; i < NUM_FILOSOFOS; i++) {
        printf("Filosofo %d comio %d veces.\n", i, comidas_hechas[i]);
        total += comidas_hechas[i];
    }
    printf("\nTotal de comidas: %d (esperado %d)\n",
           total, NUM_FILOSOFOS * COMIDAS_POR_FILOSOFO);

    sem_destroy(&sala);
    for (int i = 0; i < NUM_FILOSOFOS; i++)
        pthread_mutex_destroy(&tenedor[i]);

    return 0;
}
