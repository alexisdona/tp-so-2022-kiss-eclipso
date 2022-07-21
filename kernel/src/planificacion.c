
#include <semaphore.h>
#include <pthread.h>
#include "include/planificacion.h"
#include <commons/collections/queue.h>

uint32_t tiempo_max_bloqueo;

void iniciarPlanificacion(t_pcb* pcb, t_log* logger, int conexionCPUDispatch, int conexionMemoria) {
    pthread_mutex_lock(&mutexColaNew);
    queue_push(NEW, pcb);
    pthread_mutex_unlock(&mutexColaNew);
    iniciarPlanificacionCortoPlazo(pcb, conexionCPUDispatch, conexionMemoria, logger);
}

void iniciarPlanificacionCortoPlazo(t_pcb *pcb, int conexionCPUDispatch, int conexionMemoria, t_log* logger) {

    uint32_t atendi_dispatch = 0;
    sem_wait(&semGradoMultiprogramacion);
    pthread_mutex_lock(&mutexColaNew);
    t_pcb *pcbEnColaNew = queue_pop(NEW);
    pthread_mutex_unlock(&mutexColaNew);

    pthread_mutex_lock(&mutexColaReady);
    proceso_en_ready(pcbEnColaNew);
    pthread_mutex_unlock(&mutexColaReady);

    pthread_mutex_lock(&mutexGradoMultiprogramacion);
    GRADO_MULTIPROGRAMACION--;
    printf("GRADO_MULTIPROGRAMACION--: %d\n", GRADO_MULTIPROGRAMACION);
    pthread_mutex_unlock(&mutexGradoMultiprogramacion);

    enviarPCB(conexionCPUDispatch, queue_pop(READY), PCB); //falta definir algoritmos de planificacion

    while(conexionCPUDispatch != -1 && atendi_dispatch!=1){
        op_code cod_op = recibirOperacion(conexionCPUDispatch);
        switch(cod_op){
            case BLOQUEAR_PROCESO:
                printf("BLOQUEAR PROCESO.......\n");
                t_pcb* pcb = recibirPCB(conexionCPUDispatch);
                printf("PCB ID: %zu\n",pcb->idProceso);
                printf("PCB PC: %zu\n",pcb->programCounter);
                atendi_dispatch=1;
                bloquearProceso(pcb);
                break;
            case TERMINAR_PROCESO:
                log_info(logger, "PCB recibido para terminar proceso");
                t_pcb* pcbFinalizado = recibirPCB(conexionCPUDispatch);
                printf("pcbFinalizado->idProceso: %zu\n", pcbFinalizado->idProceso);
                printf("pcbFinalizado->tamanioProceso: %zu\n", pcbFinalizado->tamanioProceso);
                pthread_mutex_lock(&mutexGradoMultiprogramacion);
                GRADO_MULTIPROGRAMACION++;
                printf("GRADO_MULTIPROGRAMACION++: %d\n", GRADO_MULTIPROGRAMACION);
                pthread_mutex_unlock(&mutexGradoMultiprogramacion);
              //  avisarProcesoTerminado(pcbFinalizado->consola_fd);
                enviarMensaje("Proceso terminado", pcbFinalizado->consola_fd);
                 sem_post(&semGradoMultiprogramacion);
                atendi_dispatch = 1;
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

    if(pthread_mutex_init(&mutexColaBloqueados, NULL) != 0) {
        perror("Mutex cola de bloqueados fallo: ");
        error+=1;
    }
    return error;
}

void avisarProcesoTerminado(int socketDestino) {
    t_paquete* paqueteProcesoTerminado = crearPaquete();
    paqueteProcesoTerminado->codigo_operacion = TERMINAR_PROCESO;
    enviarPaquete(paqueteProcesoTerminado, socketDestino);
}

void bloquearProceso(t_pcb* pcb){

    pthread_mutex_lock(&mutexColaBloqueados);
    queue_push(BLOCKED, pcb);
    pthread_mutex_unlock(&mutexColaBloqueados);

     /* Me paro en la instruccion anterior a la que apunta el programCounter porque el ciclo de instruccion termina siempre y suma 1 al PC*/
    t_instruccion * instruccion = ((t_instruccion*) (list_get(pcb->listaInstrucciones,(pcb->programCounter)-1)));
    if (instruccion->codigo_operacion ==  IO) {
        operando tiempoBloqueado = instruccion->parametros[0];
        if(tiempoBloqueado>=TIEMPO_MAXIMO_BLOQUEADO){
            log_info(logger,"SUSPENDO EL PROCESO");
            suspenderBlockedProceso(pcb);
        }else{
        	//log_info(logger,string_from_format("BLOQUEO AL PROCESO POR %ds",(tiempoBloqueado/1000)));
        	usleep(tiempoBloqueado*1000);
        }
    }
}

void suspenderBlockedProceso(t_pcb* pcb){
    pthread_mutex_lock(&mutexColaBloqueados);
    t_pcb * pcbEnColaBlocked = queue_pop(BLOCKED);
    pthread_mutex_unlock(&mutexColaBloqueados);

    pthread_mutex_lock(&mutexColaSuspendedBloqued);
    enviarPCB(conexionMemoria,pcbEnColaBlocked,SWAPEAR_PROCESO);
    queue_push(SUSPENDED_BLOCKED, pcbEnColaBlocked);
    pthread_mutex_unlock(&mutexColaSuspendedBloqued);

    pthread_mutex_lock(&mutexGradoMultiprogramacion);
    GRADO_MULTIPROGRAMACION++;
    printf("suspenderProceso() --> GRADO_MULTIPROGRAMACION--: %d\n", GRADO_MULTIPROGRAMACION);
    pthread_mutex_unlock(&mutexGradoMultiprogramacion);
    sem_post(&semGradoMultiprogramacion);
}

void proceso_en_ready(t_pcb* pcbEnColaNew ) {
    queue_push(READY, pcbEnColaNew);
    crear_estructuras_memoria(pcbEnColaNew);
}

void crear_estructuras_memoria( t_pcb* pcb) {
    int pcb_actualizado = 0;
    t_paquete* paquete = crearPaquete();
    enviarPCB(conexionMemoria, pcb, CREAR_ESTRUCTURAS_ADMIN );
    while(conexionMemoria != -1 && pcb_actualizado == 0){
        op_code cod_op = recibirOperacion(conexionMemoria);
        switch(cod_op) {
            case ACTUALIZAR_INDICE_TABLA_PAGINAS:
                ;
                t_pcb* pcb_aux = recibirPCB(conexionMemoria);
                pcb->tablaPaginas = pcb_aux->tablaPaginas;
                printf("\nACTUALIZAR_TABLA_PAGINAS: PCB->idProceso: %ld, PCB->tablaPaginas:%ld\n", pcb->idProceso, pcb->tablaPaginas);
                pcb_actualizado=1;
                break;
        }
    }
}