#include <stdio.h>
#include <pthread.h>
#include <semaphore.h> 

#define INITIAL_SALDO 100000 // Saldo inicial de la cuenta bancaria

sem_t semaphore; // Creacion del semaforo
int SALDO_TOT = INITIAL_SALDO; // Guardar el sado inicial

void* retiro(void* id_hilo) {
    int monto;
    int cliente_id = *((int*) id_hilo);

    sem_wait(&semaphore); // Bloquea el semaforo

    printf("Cliente %d ingrese el monto a retirar: \n", cliente_id);
    scanf("%d", &monto);

    if (monto <= SALDO_TOT) {
        SALDO_TOT -= monto;
        printf("Cliente %d: Retiro exitoso. Nuevo saldo: %d\n", cliente_id, SALDO_TOT);
    } else {
        printf("Cliente %d: No hay suficientes fondos. Saldo actual: %d\n", cliente_id, SALDO_TOT);
    }

    sem_post(&semaphore); // Desbloquea el semaforo al terminar

    return NULL;
}

int main() {
    int num_clientes;

    printf("Ingrese cuántos clientes ingresarán al sistema: ");
    scanf("%d", &num_clientes); // Guarda el numero de clientes

    pthread_t ids[num_clientes];
    int cliente_ids[num_clientes]; // Genera un "id" por cada cliente

    sem_init(&semaphore, 0, 1); // Inicia el semaforo con valor 1

    for (int i = 0; i < num_clientes; i++) { // Crea los diferentes hilos
        cliente_ids[i] = i + 1; // Crea un id por cada hilo
        pthread_create(&ids[i], NULL, retiro, &cliente_ids[i]);
    }

    for (int i = 0; i < num_clientes; i++) { // Cierra todos los hilos
        pthread_join(ids[i], NULL);
    }

    sem_destroy(&semaphore); // Destruye el semaforo
    return 0;
}