# Práctica #4 - Mecanismos de sincronización en sistemas operativos

Implementación de los tres mecanismos de sincronización del laboratorio en
**C (pthreads)** y **Go (goroutines)**.

## Estructura

```
lab4/
├── c/
│   ├── queue.c                     # Tarea 1 en C
│   ├── producer_consumer.c         # Tarea 2 en C
│   └── dining_philosophers.c       # Tarea 3 en C
├── go/
│   ├── go.mod
│   ├── queue.go                     # Tarea 1 en Go
│   ├── producer_consumer.go         # Tarea 2 en Go
│   └── dining_philosophers.go       # Tarea 3 en Go
└── README.md
```

## Tarea 1: Cola segura para hilos (`queue`)

**Enfoque:** se usa `pthread_mutex_t` para garantizar exclusión mutua sobre la
cola circular y **dos** `pthread_cond_t`:

- `not_empty`: los consumidores esperan (`pthread_cond_wait`) mientras la cola
  esté vacía; `enqueue` hace `pthread_cond_signal` al agregar un elemento.
- `no_llena`: los productores esperan mientras la cola esté llena (capacidad
  10); `dequeue` señaliza al liberar espacio.

Se usa `while` (no `if`) en las esperas para evitar el problema de las
señalizaciones perdidas (spurious wakeups).

**Compilar y ejecutar (C):**

```bash
cd c
gcc -Wall -Wextra -pthread queue.c -o queue
./queue
```

**Ejecutar (Go):**

```bash
cd go && go run queue.go
```

**Salida de ejemplo:**

```
[Productor 0] encolado 0 (producidos=1, size=1)
[Consumidor 0] desencolado 0 (consumidos=1)
...
Resultado: producidos=60 consumidos=60 size_final=0
```

## Tarea 2: Productor-consumidor con semáforos (`producer_consumer`)

**Enfoque:** buffer circular acotado (`BUFFER_SIZE = 10`) con dos semáforos:

- `empty`: inicia en `BUFFER_SIZE`, cuenta espacios libres.
- `full`: inicia en 0, cuenta elementos disponibles.
- `mutex`: protege el buffer compartido (índices `entrada`/`salida`).

Orden de operaciones (evita deadlock):

- Productor: `sem_wait(&empty)` → `lock` → push → `unlock` → `sem_post(&full)`
- Consumidor: `sem_wait(&full)` → `lock` → pop → `unlock` → `sem_post(&empty)`

Así nunca hay overflow ni underflow del buffer.

**Compilar y ejecutar (C):**

```bash
cd c
gcc -Wall -Wextra -pthread producer_consumer.c -o producer_consumer
./producer_consumer
```

**Ejecutar (Go):**

```bash
cd go && go run producer_consumer.go
```

En Go el buffer acotado se implementa con un **canal con capacidad**
(`make(chan int, bufferSize)`): enviar bloquea cuando está lleno (equivale a
`sem_wait(&empty)`) y recibir bloquea cuando está vacío (equivale a
`sem_wait(&full)`).

**Salida de ejemplo:**

```
[Productor 0] produjo 0 (producidos=1, ocupados=1)
[Consumidor 0] consumio 0 (consumidos=1, ocupados=0)
...
Resultado: producidos=90 consumidos=90 ocupados_finales=0
```

## Tarea 3: Filósofos cenando (`dining_philosophers`)

**Enfoque:** solución basada en el **semáforo "sala" (room)** que limita a
`N-1` filósofos simultáneos intentando comer:

- Como a lo sumo `N-1` filósofos compiten por `N` tenedores (`tenedor[]`),
  siempre queda al menos un tenedor libre → se rompe la **espera circular** →
  no hay deadlock.
- El semáforo `sala` da espera equitativa → no hay starvation.
- Además, cada filósofo toma sus dos tenedores en **orden consistente**
  (`primero`/`segundo`), otra garantía anti-deadlock.

Cada tenedor es un `pthread_mutex_t` (C) / `sync.Mutex` (Go); la sala es un
`sem_t` con capacidad `N-1` (C) / canal con capacidad `N-1` (Go).

**Compilar y ejecutar (C):**

```bash
cd c
gcc -Wall -Wextra -pthread dining_philosophers.c -o dining_philosophers
./dining_philosophers
```

**Ejecutar (Go):**

```bash
cd go && go run dining_philosophers.go
```

**Salida de ejemplo:**

```
Filosofo 0 esta pensando...
Filosofo 0 esta comiendo...
...
Filosofo 0 comio 3 veces.
...
Total de comidas: 15 (esperado 15)
```

## Notas

- Los programas C requieren Linux/WSL con `gcc` y `pthread` (`build-essential`).
- Los programas Go son independientes por directorio (`go run ./<nombre>`).
