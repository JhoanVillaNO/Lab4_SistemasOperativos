// Tarea 3 (Go): El problema de los filosofos cenando.
//
// Solucion: canal "sala" con capacidad N-1 que limita a N-1 filosofos
// simultaneos intentando comer -> siempre queda un tenedor libre, no hay
// deadlock. Cada filosofo toma sus tenedores en orden consistente.
//
// Ejecutar: go run ./dining_philosophers
package main

import (
	"fmt"
	"math/rand"
	"sync"
	"time"
)

const (
	numFilosofos       = 5
	comidasPorFilosofo = 3
)

var (
	tenedores [numFilosofos]sync.Mutex
	sala      = make(chan struct{}, numFilosofos-1) // semaforo sala
)

func pensar(id int) {
	fmt.Printf("Filosofo %d esta pensando...\n", id)
	time.Sleep(time.Duration(200+rand.Intn(300)) * time.Millisecond)
}

func comer(id int) {
	fmt.Printf("Filosofo %d esta comiendo...\n", id)
	time.Sleep(time.Duration(100+rand.Intn(200)) * time.Millisecond)
}

func filosofo(id int, wg *sync.WaitGroup, comidas *[]int, candado *sync.Mutex) {
	defer wg.Done()

	izquierdo := id
	derecho   := (id + 1) % numFilosofos

	// Orden consistente de tenedores para evitar deadlock circular
	primero, segundo := izquierdo, derecho
	if primero > segundo {
		primero, segundo = segundo, primero
	}

	for i := 0; i < comidasPorFilosofo; i++ {
		pensar(id)

		sala <- struct{}{} // permiso para intentar comer (max N-1)

		tenedores[primero].Lock()
		tenedores[segundo].Lock()

		comer(id)

		candado.Lock()
		(*comidas)[id]++
		candado.Unlock()

		tenedores[segundo].Unlock()
		tenedores[primero].Unlock()

		<-sala
	}

	fmt.Printf("Filosofo %d termino.\n", id)
}

func main() {
	rand.Seed(time.Now().UnixNano())

	var wg sync.WaitGroup
	var candado sync.Mutex
	comidas := make([]int, numFilosofos)

	for i := 0; i < numFilosofos; i++ {
		wg.Add(1)
		go filosofo(i, &wg, &comidas, &candado)
	}
	wg.Wait()

	total := 0
	for i := 0; i < numFilosofos; i++ {
		fmt.Printf("Filosofo %d comio %d veces.\n", i, comidas[i])
		total += comidas[i]
	}
	fmt.Printf("\nTotal de comidas: %d (esperado %d)\n",
		total, numFilosofos*comidasPorFilosofo)
}
