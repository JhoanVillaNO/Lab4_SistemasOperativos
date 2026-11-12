// Tarea 2: productor-consumidor con buffer acotado usando semaforos

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE         10
#define NUM_PRODUCTORES     3
#define NUM_CONSUMIDORES    3
#define ITEMS_POR_PRODUCTOR 30

int buffer[BUFFER_SIZE];
int entrada = 0, salida = 0, ocupados = 0;

sem_t empty, full;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int producidos = 0;
static int consumidos = 0;
static pthread_mutex_t candado_estadisticas = PTHREAD_MUTEX_INITIALIZER;

static int produce_item(int id, int seq) {
    return id * 1000 + seq;
}

static void consume_item(int item) {
    (void)item;
}

static void *producer(void *arg) {
    long id = (long)arg;
    for (int i = 0; i < ITEMS_POR_PRODUCTOR; i++) {
        int item = produce_item((int)id, i);

        sem_wait(&empty);   // Esperar si el buffer esta lleno
        pthread_mutex_lock(&mutex);
        // Agregar elemento al buffer
        buffer[entrada] = item;
        entrada = (entrada + 1) % BUFFER_SIZE;
        ocupados++;
        pthread_mutex_unlock(&mutex);
        sem_post(&full);    // Senalar que hay un elemento disponible

        pthread_mutex_lock(&candado_estadisticas);
        producidos++;
        int p = producidos;
        int occ = ocupados;
        pthread_mutex_unlock(&candado_estadisticas);

        printf("[Productor %ld] produjo %d (producidos=%d, ocupados=%d)\n",
               id, item, p, occ);
        usleep(800);
    }
    return NULL;
}

static void *consumer(void *arg) {
    long id = (long)arg;
    int esperados = NUM_PRODUCTORES * ITEMS_POR_PRODUCTOR / NUM_CONSUMIDORES;
    for (int i = 0; i < esperados; i++) {
        sem_wait(&full);    // Esperar si el buffer esta vacio
        pthread_mutex_lock(&mutex);
        // Quitar elemento del buffer
        int item = buffer[salida];
        salida = (salida + 1) % BUFFER_SIZE;
        ocupados--;
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);   // Senalar que hay un espacio libre

        consume_item(item);

        pthread_mutex_lock(&candado_estadisticas);
        consumidos++;
        int c = consumidos;
        int occ = ocupados;
        pthread_mutex_unlock(&candado_estadisticas);

        printf("[Consumidor %ld] consumio %d (consumidos=%d, ocupados=%d)\n",
               id, item, c, occ);
        usleep(1200);
    }
    return NULL;
}

int main(void) {
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);

    pthread_t prod[NUM_PRODUCTORES], cons[NUM_CONSUMIDORES];

    for (long i = 0; i < NUM_PRODUCTORES; i++)
        pthread_create(&prod[i], NULL, producer, (void *)i);
    for (long i = 0; i < NUM_CONSUMIDORES; i++)
        pthread_create(&cons[i], NULL, consumer, (void *)i);

    for (int i = 0; i < NUM_PRODUCTORES; i++)
        pthread_join(prod[i], NULL);
    for (int i = 0; i < NUM_CONSUMIDORES; i++)
        pthread_join(cons[i], NULL);

    printf("\nResultado: producidos=%d consumidos=%d ocupados_finales=%d\n",
           producidos, consumidos, ocupados);

    sem_destroy(&empty);
    sem_destroy(&full);
    return 0;
}
