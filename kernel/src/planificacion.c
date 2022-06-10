
#include <semaphore.h>
#include <pthread.h>
#include "include/planificacion.h"
#include <commons/collections/queue.h>

uint32_t tiempo_max_bloqueo;

int iniciarPlanificacion(t_pcb* pcb, t_log* logger, int conexionCPUDispatch) {
    //tiempo_max_bloqueo = tiempo_bloqueo_config;
	log_info(logger, "Iniciando planificación");
    pthread_mutex_lock(&mutexColaNew);
    queue_push(NEW, pcb);
    printf("dentro de mutex de cola de new. Tamaño de la cola de NEW: %d\n", queue_size(NEW));
    pthread_mutex_unlock(&mutexColaNew);
    iniciarPlanificacionCortoPlazo(pcb, conexionCPUDispatch);
}

void iniciarPlanificacionCortoPlazo(t_pcb *pcb, int conexionCPUDispatch) {
    uint32_t atendi_dispatch = 0;

	printf("Entra en inciarPlanificacion de corto plazo\n");

    sem_wait(&semGradoMultiprogramacion);
    sem_getvalue(&semGradoMultiprogramacion, &valorSemaforoContador);
    printf("planificacion -> Valor semaforo contador: %d\n",valorSemaforoContador );

    pthread_mutex_lock(&mutexColaReady);
    queue_push(READY, queue_pop(NEW));
    printf("dentro de mutex de cola de ready: tamaño cola de ready: %d\n", queue_size(READY));
    GRADO_MULTIPROGRAMACION--;
    printf("GRADO_MULTIPROGRAMACION: %d\n", GRADO_MULTIPROGRAMACION);
    pthread_mutex_unlock(&mutexColaReady);
    enviarPCB(conexionCPUDispatch, queue_pop(READY) ); //falta definir algoritmos de planificacion

    while(conexionCPUDispatch != -1 && atendi_dispatch!=1){
    	op_code cod_op = recibirOperacion(conexionCPUDispatch);
    	switch(cod_op){
    		case BLOQUEAR_PROCESO:
    			printf("BLOQUEAR PROCESO.......\n");
    			t_pcb* pcb = recibirPCB(conexionCPUDispatch);
    			printf("PCB ID: %d\n",pcb->idProceso);
    			printf("PCB PC: %d\n",pcb->programCounter);
    			atendi_dispatch=1;
    			//bloquear_proceso(pcb);
    			break;
    		case TERMINAR_PROCESO:
				printf("TERMINAR PROCESO.......\n");
				atendi_dispatch=1;
				break;
    		default:
				printf("Cod_op=%d\n",cod_op);
    	}
    }
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

void bloquear_proceso(t_pcb* pcb, uint32_t tiempo_bloqueo){

}
