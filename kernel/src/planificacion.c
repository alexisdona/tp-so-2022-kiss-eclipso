
#include <semaphore.h>
#include <pthread.h>
#include "include/planificacion.h"
#include <commons/collections/queue.h>

void iniciarPlanificacion(t_pcb* pcb, t_log* logger, int conexionCPUDispatch) {
   // log_info(logger, "Iniciando planificación");
    pthread_mutex_lock(&mutexColaNew);
    queue_push(NEW, pcb);
  //  printf("dentro de mutex de cola de new. Tamaño de la cola de NEW: %d\n", queue_size(NEW));
    pthread_mutex_unlock(&mutexColaNew);
    iniciarPlanificacionCortoPlazo(pcb, conexionCPUDispatch);

}

void iniciarPlanificacionCortoPlazo(t_pcb *pcb, int conexionCPUDispatch) {
  //  printf("Entra en inciarPlanificacion de corto plazo\n");

    sem_wait(&semGradoMultiprogramacion);
    sem_getvalue(&semGradoMultiprogramacion, &valorSemaforoContador);
  //  printf("planificacion -> Valor semaforo contador: %d\n",valorSemaforoContador );

    pthread_mutex_lock(&mutexColaReady);
    queue_push(READY, queue_pop(NEW));
    pthread_mutex_unlock(&mutexColaReady);
   // printf("dentro de mutex de cola de ready: tamaño cola de ready: %d\n", queue_size(READY));
    pthread_mutex_lock(&mutexGradoMultiprogramacion);
    GRADO_MULTIPROGRAMACION--;
    printf("GRADO_MULTIPROGRAMACION--: %d\n", GRADO_MULTIPROGRAMACION);
    pthread_mutex_unlock(&mutexGradoMultiprogramacion);
    enviarPCB(conexionCPUDispatch, queue_pop(READY), PCB ); //falta definir algoritmos de planificacion
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
    if(pthread_mutex_init(&mutexGradoMultiprogramacion, NULL) != 0) {
        perror("Mutex grado de multiprogramación fallo: ");
        error+=1;
    }
    return error;
}