// Tarea 1 (Go): cola segura para hilos con sync.Mutex y sync.Cond
package main

import (
	"fmt"
	"sync"
	"time"
)

const (
	capacidadCola     = 10
	numProductores    = 3
	numConsumidores   = 3
	itemsPorProductor = 20
)

type ThreadSafeQueue struct {
	items    []int
	front    int
	rear     int
	size     int
	capacity int

	mu       sync.Mutex
	notEmpty *sync.Cond
	notFull  *sync.Cond
}

func NewThreadSafeQueue(capacity int) *ThreadSafeQueue {
	q := &ThreadSafeQueue{
		items:    make([]int, capacity),
		capacity: capacity,
	}
	q.notEmpty = sync.NewCond(&q.mu)
	q.notFull = sync.NewCond(&q.mu)
	return q
}

func (q *ThreadSafeQueue) empty() bool { return q.size == 0 }
func (q *ThreadSafeQueue) full() bool  { return q.size == q.capacity }

func (q *ThreadSafeQueue) Enqueue(item int) {
	q.mu.Lock()
	for q.full() {
		q.notFull.Wait()
	}
	// Agregar elemento a la cola
	q.items[q.rear] = item
	q.rear = (q.rear + 1) % q.capacity
	q.size++
	q.notEmpty.Signal()
	q.mu.Unlock()
}

func (q *ThreadSafeQueue) Dequeue() int {
	q.mu.Lock()
	for q.empty() {
		q.notEmpty.Wait()
	}
	// Quitar elemento de la cola
	item := q.items[q.front]
	q.front = (q.front + 1) % q.capacity
	q.size--
	q.notFull.Signal()
	q.mu.Unlock()
	return item
}

func producer(q *ThreadSafeQueue, id int, wg *sync.WaitGroup, candado *sync.Mutex, producidos *int) {
	defer wg.Done()
	for i := 0; i < itemsPorProductor; i++ {
		item := id*1000 + i
		q.Enqueue(item)
		candado.Lock()
		*producidos++
		p := *producidos
		tam := q.size
		candado.Unlock()
		fmt.Printf("[Productor %d] encolo %d (producidos=%d, size=%d)\n", id, item, p, tam)
		time.Sleep(time.Millisecond)
	}
}

func consumer(q *ThreadSafeQueue, id int, wg *sync.WaitGroup, candado *sync.Mutex, consumidos *int) {
	defer wg.Done()
	esperados := numProductores * itemsPorProductor / numConsumidores
	for i := 0; i < esperados; i++ {
		item := q.Dequeue()
		candado.Lock()
		*consumidos++
		c := *consumidos
		candado.Unlock()
		fmt.Printf("[Consumidor %d] desencolo %d (consumidos=%d)\n", id, item, c)
		time.Sleep(1500 * time.Microsecond)
	}
}

func main() {
	cola := NewThreadSafeQueue(capacidadCola)

	var wg sync.WaitGroup
	var candado sync.Mutex
	producidos, consumidos := 0, 0

	for i := 0; i < numProductores; i++ {
		wg.Add(1)
		go producer(cola, i, &wg, &candado, &producidos)
	}
	for i := 0; i < numConsumidores; i++ {
		wg.Add(1)
		go consumer(cola, i, &wg, &candado, &consumidos)
	}
	wg.Wait()

	fmt.Printf("\nResultado: producidos=%d consumidos=%d size_final=%d\n",
		producidos, consumidos, cola.size)
}
