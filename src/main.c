#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../include/process.h"
#include "../include/resource.h"
#include "../include/scheduler.h"
#include "../include/ipc.h"
#include "../include/logger.h"
#include "../include/server.h"

// ─── Estructura principal del simulador ──────────────────────────────────────
typedef struct {
    RecursoPool* recursos;
    Scheduler*   scheduler;
    Logger*      logger;
    PCB*         procesos[100];
    int          total_procesos;
} Simulador;

// ─── Hilo del servidor HTTP ───────────────────────────────────────────────────
void* hilo_servidor(void* arg) {
    servidor_iniciar(arg);
    return NULL;
}

// ─── Demo productor-consumidor ────────────────────────────────────────────────
Buffer* buffer_global = NULL;

void* hilo_productor(void* arg) {
    int pid = 1;
    for (int i = 1; i <= 5; i++) {
        buffer_producir(buffer_global, i, pid);
        // pequeña pausa simulada
        struct timespec ts = {0, 500000000}; // 0.5 seg
        nanosleep(&ts, NULL);
    }
    return NULL;
}

void* hilo_consumidor(void* arg) {
    int pid = 2;
    for (int i = 1; i <= 5; i++) {
        buffer_consumir(buffer_global, pid);
        struct timespec ts = {0, 800000000}; // 0.8 seg
        nanosleep(&ts, NULL);
    }
    return NULL;
}

// ─── Punto de entrada ─────────────────────────────────────────────────────────
int main() {
    Simulador* sim = (Simulador*)malloc(sizeof(Simulador));
    sim->recursos       = recursos_crear();
    sim->scheduler      = scheduler_crear(ROUND_ROBIN, 4);
    sim->logger         = logger_crear();
    sim->total_procesos = 0;

    logger_registrar(sim->logger, -1, "SISTEMA", EVT_SISTEMA,
                     "Simulador iniciado");

    // Demo productor-consumidor — corre primero
    buffer_global = buffer_crear();
    pthread_t prod, cons;
    pthread_create(&prod, NULL, hilo_productor, NULL);
    pthread_create(&cons, NULL, hilo_consumidor, NULL);
    
    // Esperar 5 segundos para ver el demo antes de arrancar el servidor
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);


    // Arrancar servidor en hilo separado
    pthread_t hilo;
    pthread_create(&hilo, NULL, hilo_servidor, sim);

    printf("=== Simulador de Gestor de Procesos ===\n");
   printf("Interfaz web en: http://localhost:3000\n");
    printf("Abre esa URL en tu navegador\n");
    printf("Presiona Ctrl+C para salir\n\n");

    // Mantener el programa corriendo
    pthread_join(hilo, NULL);

    return 0;
}