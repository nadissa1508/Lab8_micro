/*
 * UNIVERSIDAD DEL VALLE DE GUATEMALA
 * CC3086 - Programacion de Microprocesadores
 * Ver.: 1.1 2024/10/12
 * Mod.: Jorge Luis Felipe Aguilar Portillo
 * Mod.: Angie Nadissa Vela López
 * Carnet: 23764
 *
 * -------------------------------------------------------------
 * Programa que simula el funcionamiento de una fábrica de 
 * piezas y ensamblaje de sillas
 * -------------------------------------------------------------
 */

#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

const char* productos[] = {"Pata", "Respaldo", "Asiento", "Pata", "Pata"};
const int numProductos = 5;  
const int MAX_BUFFER = 5;    
const int MAX_SILLAS = 3;    

int buffer[MAX_BUFFER];       
int in = 0;                   
int out = 0;                  
int sillasProducidas = 0;    
bool produccionFinalizada = false; // Bandera para indicar si se han producido todas las sillas necesarias

// Semáforos y mutex
sem_t vacios;   
sem_t llenos;   
pthread_mutex_t mutex;

// Función  simulación de un productor (fabrica una pieza de silla)
void* productor(void* arg) {
    int id = *(int*)arg;
    int piezaId;
    
    while (sillasProducidas < MAX_SILLAS) {
        // Bloqueo mutex para verificar el estado de producción
        pthread_mutex_lock(&mutex);
        // Si la producción ya se ha finalizado, romper el bucle
        if (produccionFinalizada) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);
        
        piezaId = rand() % numProductos;  	// Seleccionar una pieza  al azar

        sem_wait(&vacios);  				// Espera hasta que hay espacio en el buffer
        pthread_mutex_lock(&mutex);  		// Protege el acceso al buffer

        //verificar si la producción ha finalizado después de obtener espacio en el buffer
        if (produccionFinalizada) {
            pthread_mutex_unlock(&mutex);
            sem_post(&vacios);  //liberar espacio en el buffer
            break;
        }

        buffer[in] = piezaId;				// Añade la pieza al buffer
        cout << "Productor " << id << " ha fabricado la pieza " << productos[piezaId]
             << " y la coloco en la posicion " << in << endl;
        in = (in + 1) % MAX_BUFFER;  		// Avanza el índice circular del buffer

        pthread_mutex_unlock(&mutex);
        sem_post(&llenos);  				// Incrementa el número de productos disponibles
        
        sleep(1);  							// Simula el tiempo de fabricación
    }
    
    return NULL;
}

// Función simulación de un consumidor (ensambla una silla)
void* consumidor(void* arg) {
    int id = *(int*)arg;
    int piezaId;

    while (sillasProducidas < MAX_SILLAS) {
        sem_wait(&llenos);  				// Espera hasta que existan productos disponibles
        pthread_mutex_lock(&mutex);  		// Protege el acceso al buffer

        // Verificar si ya se alcanzó el límite de sillas
        if (sillasProducidas >= MAX_SILLAS) {
            produccionFinalizada = true; //ya se completaron las sillas necesarias
            pthread_mutex_unlock(&mutex);
            sem_post(&llenos);  
            break;
        }

        // Retirar una pieza del buffer
        piezaId = buffer[out];
        cout << "Consumidor " << id << " ha retirado la pieza " << productos[piezaId]
             << " de la posicion " << out << endl;
        out = (out + 1) % MAX_BUFFER;  		// Avanza en el índice circular del buffer

        // Incrementa el contador de sillas ensambladas cuando se consumen todas las piezas necesarias
        if (piezaId == numProductos - 1) {
            sillasProducidas++;
            cout << "Consumidor " << id << " ha ensamblado una silla completa. Sillas ensambladas: "
                 << sillasProducidas << "/" << MAX_SILLAS << endl;
        }

        pthread_mutex_unlock(&mutex);
        sem_post(&vacios);  				// Incrementa el número de espacios vacíos
        
        sleep(2);  							// Simula el tiempo de ensamblaje
    }

    return NULL;
}

//funcion para generar el reporte final
void reporte_final(){
     int cantPatas = 0, cantRespaldos = 0, cantAsientos = 0;

    //Encontrar las piezas restantes en el almacen
    for (int i = 0; i < MAX_BUFFER; ++i) {
        if(buffer[i] == 0 || buffer[i] == 3 || buffer[i] == 4 ){
            cantPatas++;
        }else if(buffer[i] == 1){
            cantRespaldos++;
        }else if(buffer[i] == 2){
             cantAsientos++;
        }//{"Pata", "Respaldo", "Asiento", 
    }
    cout << "\n*****************************";
    cout << "\nReporte final \nTotal de sillas producidas: " << sillasProducidas << "\n\nPiezas del almacen restantes:" ;
    cout << "\nPatas: " << cantPatas << "\nRespaldos: " << cantRespaldos << "\nAsientos: " << cantAsientos << endl;
}

int main() {
    int numProductores, numConsumidores;

    // Solicitar la cantidad de productores y consumidores
    cout << "Ingrese el numero de productores: ";
    cin >> numProductores;
    cout << "Ingrese el numero de consumidores: ";
    cin >> numConsumidores;

    pthread_t productores[100], consumidores[100];  
    int idProductores[100], idConsumidores[100];    

    // Inicializa semáforos y mutex
    sem_init(&vacios, 0, MAX_BUFFER);  
    sem_init(&llenos, 0, 0);         
    pthread_mutex_init(&mutex, NULL);

    // Crea hilos productores
    for (int i = 0; i < numProductores; ++i) {
        idProductores[i] = i + 1;
        pthread_create(&productores[i], NULL, productor, &idProductores[i]);
    }

    // Crea hilos consumidores
    for (int i = 0; i < numConsumidores; ++i) {
        idConsumidores[i] = i + 1;
        pthread_create(&consumidores[i], NULL, consumidor, &idConsumidores[i]);
    }

    // Espera a que los hilos terminen
    for (int i = 0; i < numProductores; ++i) {
        pthread_join(productores[i], NULL);
    }

    for (int i = 0; i < numConsumidores; ++i) {
        pthread_join(consumidores[i], NULL);
    }

    reporte_final();

    // Destruye semáforos y mutex
    sem_destroy(&vacios);
    sem_destroy(&llenos);
    pthread_mutex_destroy(&mutex);

    return 0;
}
