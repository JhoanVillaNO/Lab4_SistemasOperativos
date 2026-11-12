/*
 * Tarea 1: Implementacion de una cola segura para hilos usando
 * locks y variables de condicion.
 *
 * Compilar: gcc -Wall -Wextra -pthread queue.c -o queue
 * Ejecutar:  ./queue
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CAPACIDAD_COLA      10
#define NUM_PRODUCTORES     3
#define NUM_CONSUMIDORES    3
#define ITEMS_POR_PRODUCTOR 20

typedef struct {
    int *items;
    int front, rear, size;
    int capacidad;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t no_llena;
} ThreadSafeQueue;

static ThreadSafeQueue cola;
static int producidos = 0;
static int consumidos = 0;
static pthread_mutex_t candado_estadisticas = PTHREAD_MUTEX_INITIALIZER;

static int queue_empty(ThreadSafeQueue *q) {
    return q->size == 0;
}

static int cola_llena(ThreadSafeQueue *q) {
    return q->size == q->capacidad;
}

static void cola_init(ThreadSafeQueue *q, int capacidad) {
    q->items = malloc(sizeof(int) * capacidad);
    q->front = q->rear = q->size = 0;
    q->capacidad = capacidad;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->no_llena, NULL);
}

static void cola_destroy(ThreadSafeQueue *q) {
    free(q->items);
    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->not_empty);
    pthread_cond_destroy(&q->no_llena);
}

void enqueue(ThreadSafeQueue *q, int item) {
    pthread_mutex_lock(&q->lock);
    while (cola_llena(q)) {
        pthread_cond_wait(&q->no_llena, &q->lock);
    }
    // Agregar elemento a la cola
    q->items[q->rear] = item;
    q->rear = (q->rear + 1) % q->capacidad;
    q->size++;
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
}

int dequeue(ThreadSafeQueue *q) {
    pthread_mutex_lock(&q->lock);
    while (queue_empty(q)) {
        pthread_cond_wait(&q->not_empty, &q->lock);
    }
    // Quitar elemento de la cola
    int item = q->items[q->front];
    q->front = (q->front + 1) % q->capacidad;
    q->size--;
    pthread_cond_signal(&q->no_llena);
    pthread_mutex_unlock(&q->lock);
    return item;
}

static void *producer(void *arg) {
    long id = (long)arg;
    for (int i = 0; i < ITEMS_POR_PRODUCTOR; i++) {
        int item = (int)(id * 1000 + i);
        enqueue(&cola, item);
        pthread_mutex_lock(&candado_estadisticas);
        producidos++;
        int p = producidos;
        int tam = cola.size;
        pthread_mutex_unlock(&candado_estadisticas);
        printf("[Productor %ld] encolo %d (producidos=%d, size=%d)\n", id, item, p, tam);
        usleep(1000);
    }
    return NULL;
}

static void *consumer(void *arg) {
    long id = (long)arg;
    int esperados = NUM_PRODUCTORES * ITEMS_POR_PRODUCTOR / NUM_CONSUMIDORES;
    for (int i = 0; i < esperados; i++) {
        int item = dequeue(&cola);
        pthread_mutex_lock(&candado_estadisticas);
        consumidos++;
        int c = consumidos;
        pthread_mutex_unlock(&candado_estadisticas);
        printf("[Consumidor %ld] desencolo %d (consumidos=%d)\n", id, item, c);
        usleep(1500);
    }
    return NULL;
}

int main(void) {
    cola_init(&cola, CAPACIDAD_COLA);

    pthread_t prod[NUM_PRODUCTORES], cons[NUM_CONSUMIDORES];

    for (long i = 0; i < NUM_PRODUCTORES; i++)
        pthread_create(&prod[i], NULL, producer, (void *)i);
    for (long i = 0; i < NUM_CONSUMIDORES; i++)
        pthread_create(&cons[i], NULL, consumer, (void *)i);

    for (int i = 0; i < NUM_PRODUCTORES; i++)
        pthread_join(prod[i], NULL);
    for (int i = 0; i < NUM_CONSUMIDORES; i++)
        pthread_join(cons[i], NULL);

    printf("\nResultado: producidos=%d consumidos=%d size_final=%d\n",
           producidos, consumidos, cola.size);

    cola_destroy(&cola);
    return 0;
}
