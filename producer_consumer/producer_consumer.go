// Tarea 2 (Go): Problema productor-consumidor con buffer acotado.
//
// Un canal con capacidad actua como buffer acotado:
//   - Enviar bloquea cuando esta lleno  -> sem empty (espacios).
//   - Recibir bloquea cuando esta vacio -> sem full (ocupados).
//
// Ejecutar: go run ./producer_consumer
package main

import (
	"fmt"
	"sync"
	"time"
)

const (
	bufferSize        = 10
	numProductores    = 3
	numConsumidores   = 3
	itemsPorProductor = 30
)

var buffer = make(chan int, bufferSize)

func produce_item(id, seq int) int {
	return id*1000 + seq
}

func consume_item(item int) {
	_ = item
}

func producer(id int, wg *sync.WaitGroup, candado *sync.Mutex, producidos *int) {
	defer wg.Done()
	for i := 0; i < itemsPorProductor; i++ {
		item := produce_item(id, i)

		buffer <- item // bloquea si el buffer esta lleno (sem_wait(&empty))

		candado.Lock()
		*producidos++
		p := *producidos
		ocupados := len(buffer)
		candado.Unlock()
		fmt.Printf("[Productor %d] produjo %d (producidos=%d, ocupados=%d)\n",
			id, item, p, ocupados)
		time.Sleep(800 * time.Microsecond)
	}
}

func consumer(id int, wg *sync.WaitGroup, candado *sync.Mutex, consumidos *int) {
	defer wg.Done()
	esperados := numProductores * itemsPorProductor / numConsumidores
	for i := 0; i < esperados; i++ {
		item := <-buffer // bloquea si el buffer esta vacio (sem_wait(&full))

		consume_item(item)

		candado.Lock()
		*consumidos++
		c := *consumidos
		ocupados := len(buffer)
		candado.Unlock()
		fmt.Printf("[Consumidor %d] consumio %d (consumidos=%d, ocupados=%d)\n",
			id, item, c, ocupados)
		time.Sleep(1200 * time.Microsecond)
	}
}

func main() {
	var wg sync.WaitGroup
	var candado sync.Mutex
	producidos, consumidos := 0, 0

	for i := 0; i < numProductores; i++ {
		wg.Add(1)
		go producer(i, &wg, &candado, &producidos)
	}
	for i := 0; i < numConsumidores; i++ {
		wg.Add(1)
		go consumer(i, &wg, &candado, &consumidos)
	}
	wg.Wait()

	fmt.Printf("\nResultado: producidos=%d consumidos=%d ocupados_finales=%d\n",
		producidos, consumidos, len(buffer))
}
