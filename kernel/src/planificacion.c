
#include <semaphore.h>
#include <pthread.h>
#include "include/planificacion.h"
#include <commons/collections/queue.h>

uint32_t tiempo_max_bloqueo;
t_pcb* pcb_en_ejecucion;

void iniciarPlanificacion(t_pcb* pcb, int conexionCPUDispatch, char* algoritmoPlanificacion, t_log* logger) {
   // log_info(logger, "Iniciando planificación");
    pthread_mutex_lock(&mutexColaNew);
    queue_push(NEW, pcb);
  //  printf("dentro de mutex de cola de new. Tamaño de la cola de NEW: %d\n", queue_size(NEW));
    pthread_mutex_unlock(&mutexColaNew);
    iniciarPlanificacionCortoPlazo(pcb, conexionCPUDispatch, algoritmoPlanificacion, logger);
}

void iniciarPlanificacionCortoPlazo(t_pcb *pcb, int conexionCPUDispatch, char* algoritmoPlanificacion, t_log* logger) {
    //  printf("Entra en inciarPlanificacion de corto plazo\n");
    //tiempo_max_bloqueo = tiempo_bloqueo_config;

    uint32_t atendi_dispatch = 0;
    sem_wait(&semGradoMultiprogramacion);

    pthread_mutex_lock(&mutexColaNew);
    t_pcb *pcbEnColaNew = queue_pop(NEW);
    pthread_mutex_unlock(&mutexColaNew);

    pthread_mutex_lock(&mutexColaReady);

    list_add(READY, pcbEnColaNew);
    
    iniciar_algoritmo_planificacion(algoritmoPlanificacion);
  
    pthread_mutex_unlock(&mutexColaReady);

    pthread_mutex_lock(&mutexGradoMultiprogramacion);
    GRADO_MULTIPROGRAMACION--;
    printf("GRADO_MULTIPROGRAMACION--: %d\n", GRADO_MULTIPROGRAMACION);
    pthread_mutex_unlock(&mutexGradoMultiprogramacion);

    pcb_en_ejecucion = obtener_proceso_en_READY();
    enviarPCB(conexionCPUDispatch, pcb_en_ejecucion, PCB); 
    time_t tiempo_en_ejecucion = time(NULL);

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
                time_t tiempo_finalizacion = time(NULL)
                tiempo_en_ejecucion = tiempo_en_ejecucion - tiempo_finalizacion
                estimar_proxima_rafaga(tiempo_en_ejecucion, pcb);
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
            case DESALOJAR_PROCESO:
                t_pcb* pcb = recibirPCB(conexionCPUDispatch);
                checkear_proceso(pcb);
                break;
            default:
                printf("Cod_op=%d\n",cod_op);
        }
    }
}

void iniciar_algoritmo_planificacion(char* algoritmoPlanificacion) {
     if (list_size(READY) > 0) {
        if (strcmp("SJF", algoritmoPlanificacion)) {
            //Me llega un PCB, entonces:
            //Reordeno LISTA READY 
            //Chequeo si el PCB en la primer posicion es mas corto que el PCB en EXEC -> interrumpir cpu mandando PCB corto
            checkear_proceso_rafagas(pcb_en_ejecucion);
        }
    }
}

void replanificar_READY(t_pcb* pcb) {
    list_add(READY, pcb);
    ordenar_procesos_lista_READY();
}

t_pcb* obtener_proceso_en_READY() {
    return list_get(READY, 0);
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
    if (instruccion->codigo_operacion == IO) {
        operando tiempoBloqueado = instruccion->parametros[0];
        if(tiempoBloqueado>=TIEMPO_MAXIMO_BLOQUEADO){
            suspenderBlockedProceso(pcb);
        };
    }
}

void suspenderBlockedProceso(t_pcb* pcb){
    pthread_mutex_lock(&mutexColaBloqueados);
    t_pcb * pcbEnColaBlocked = queue_pop(BLOCKED);
    pthread_mutex_unlock(&mutexColaBloqueados);

    pthread_mutex_lock(&mutexColaSuspendedBloqued);
    queue_push(SUSPENDED_BLOCKED, pcbEnColaBlocked);
    pthread_mutex_unlock(&mutexColaSuspendedBloqued);

    pthread_mutex_lock(&mutexGradoMultiprogramacion);
    GRADO_MULTIPROGRAMACION++;
    printf("suspenderProceso() --> GRADO_MULTIPROGRAMACION--: %d\n", GRADO_MULTIPROGRAMACION);
    pthread_mutex_unlock(&mutexGradoMultiprogramacion);
    sem_post(&semGradoMultiprogramacion);
}

 // si el proceso A es el seleccionado para ejecutar (ya que tiene menos ráfaga que el B y el C), 
 // ya se sabe que entre el A, B y C siempre el A va a ser el que tenga mas prioridad. 
// hacer la comparación de ráfagas solo en el caso de que entre un nuevo proceso a ready.

void estimar_proxima_rafaga(time_t tiempo){
	int tiempo_cpu = tiempo / 1000;
	pcb->estimacionRafaga = alpha*tiempo_cpu + (1-alpha)*(pcb->estimacionRafaga);
}

void ordenar_procesos_lista_READY() {
    // Algoritmo SJF - ordenamiento
    // t_pcb* pcb_aux = malloc(sizeof(t_pcb));

    // if(!pcb_aux){
    //     printf("No se pudo reservar memoria para el PCB\n");
    //     return 0;
    // }
   list_sort(READY, sort_by_rafaga);

   printf("----- LISTA REORDENADA ----")
   for (int i=0; i < list_size(READY); i++){
        t_pcb* pcb = list_get(pcb, i);
        printf("pcb->idProceso->%d\testimacionRafaga-> %d\n",
               pcb->idProceso,
               pcb->estimacionRafaga
    }
}

static bool sort_by_rafaga(void* pcb1, void* pcb2) {
    return (((t_pcb*) pcb1)->estimacionRafaga) < (((t_pcb*) pcb2)->estimacionRafaga);
}

void checkear_proceso(pcbEnExec) {
    t_pcb* pcb = obtener_proceso_en_READY();
    if(pcb->estimacionRafaga < pcbEnExec->estimacionRafaga) { 
       interrumpir_proceso_en_CPU(pcbEnExec)
       replanificar_y_enviar_nuevo_proceso(pcb, pcbEnExec, conexionCPUDispatch);
    }
}
void actualizar_proceso_bloqueado_rafagas() {

}

interrumpir_proceso_en_CPU() {
  
}

void replanificar_y_enviar_nuevo_proceso(pcbNueva, pcbEnExec, conexionCPUDispatch) {
    enviarPCB(conexionCPUDispatch, pcbNueva, PCB); 
    replanificar_READY(pcbExec);
}
