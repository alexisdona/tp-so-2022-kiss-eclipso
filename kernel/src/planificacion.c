
#include <semaphore.h>
#include <pthread.h>
#include "include/planificacion.h"
#include <commons/collections/queue.h>

int iniciarPlanificacion(t_pcb* pcb, t_log* logger, int conexionCPUDispatch) {
    log_info(logger, "Iniciando planificación");
    pthread_mutex_lock(&mutexColaNew);
    queue_push(NEW, pcb);
    printf("dentro de mutex de cola de new\n");
    pthread_mutex_unlock(&mutexColaNew);
    iniciarPlanificacionCortoPlazo(pcb, conexionCPUDispatch);


}

void iniciarPlanificacionCortoPlazo(t_pcb *pcb, int conexionCPUDispatch) {
    printf("Entra en inciarPlanificacion de corto plazo\n");

    sem_wait(&semGradoMultiprogramacion);
    pthread_mutex_lock(&mutexColaReady);
    queue_push(READY, queue_pop(NEW));
    printf("dentro de mutex de cola de ready: tamaño cola de ready: %d\n", queue_size(READY));
    GRADO_MULTIPROGRAMACION--;
    printf("GRADO_MULTIPROGRAMACION: %d\n", GRADO_MULTIPROGRAMACION);
    pthread_mutex_unlock(&mutexColaReady);
    enviarPCB(conexionCPUDispatch, queue_pop(READY) ); //falta definir algoritmos de planificacion
}


int inicializarMutex() {
    int error=0;
    if(pthread_mutex_init(&mutexColaNew, NULL) != 0) {
        perror("Mutex cola de new fallo: ");
        error+=1;
    }
    if(pthread_mutex_init(&mutexColaReady, NULL) != 0) {
        perror("Mutex cola de ready fallo: ");
        error+=1;
    }
    return error;
}