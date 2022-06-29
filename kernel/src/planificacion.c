
#include <semaphore.h>
#include <pthread.h>
#include "include/planificacion.h"
#include <commons/collections/queue.h>

double ALFA;

void iniciarPlanificacion(t_attrs_planificacion* attrs_planificacion) {
    log_info(attrs_planificacion->logger, "Iniciando planificación");
    pthread_mutex_lock(&mutexColaNew);
    queue_push(NEW, attrs_planificacion->pcb);
  //  printf("dentro de mutex de cola de new. Tamaño de la cola de NEW: %d\n", queue_size(NEW));
    pthread_mutex_unlock(&mutexColaNew);
    iniciarPlanificacionCortoPlazo(attrs_planificacion);
}

void iniciarPlanificacionCortoPlazo(t_attrs_planificacion* attrs_planificacion) {
    //  printf("Entra en inciarPlanificacion de corto plazo\n");
    tiempo_max_bloqueo = attrs_planificacion->tiempo_maximo_bloqueado;
    ALFA = attrs_planificacion->alpha;
    conexionCPUDispatch = attrs_planificacion->conexion_cpu_dispatch;
    conexionCPUInterrupt = attrs_planificacion->conexion_cpu_interrupt;

    uint32_t atendi_dispatch = 0;
    sem_wait(&semGradoMultiprogramacion);

    pthread_mutex_lock(&mutexColaNew);
    t_pcb *pcbEnColaNew = queue_pop(NEW);
    pthread_mutex_unlock(&mutexColaNew);

    pthread_mutex_lock(&mutexColaReady);

    iniciar_algoritmo_planificacion(attrs_planificacion->algoritmo_planificacion, pcbEnColaNew);
  
    pthread_mutex_unlock(&mutexColaReady);

    time_t tiempo_inicial = time(NULL);
    
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
                time_t tiempo_en_ejecucion = calcular_tiempo_en_exec(tiempo_inicial);
                estimar_proxima_rafaga(tiempo_en_ejecucion, pcb);
                break;
            case TERMINAR_PROCESO:
                log_info(attrs_planificacion->logger, "PCB recibido para terminar proceso");
                t_pcb* pcbFinalizado = recibirPCB(conexionCPUDispatch);
                printf("pcbFinalizado->idProceso: %zu\n", pcbFinalizado->idProceso);
                printf("pcbFinalizado->tamanioProceso: %zu\n", pcbFinalizado->tamanioProceso);
                incrementar_grado_multiprogramacion();
              //  avisarProcesoTerminado(pcbFinalizado->consola_fd);
                enviarMensaje("Proceso terminado", pcbFinalizado->consola_fd);
                sem_post(&semGradoMultiprogramacion);
                atendi_dispatch = 1;
                break;
            case DESALOJAR_PROCESO:
            	log_info(attrs_planificacion->logger, "PCB recibido desalojado por la CPU");
                t_pcb* pcb_desalojada = recibirPCB(conexionCPUDispatch);
                tiempo_en_ejecucion = calcular_tiempo_en_exec(tiempo_inicial);
                calcular_rafagas_restantes_proceso_desalojado(tiempo_inicial, tiempo_en_ejecucion, pcb_desalojada);
                checkear_proceso_y_replanificar(pcb_desalojada);
                break;
            default:
                printf("Cod_op=%d\n", cod_op);
        }
    }
}

void eliminar_proceso_de_READY() {
    list_remove(READY, 0);
}

time_t calcular_tiempo_en_exec(time_t tiempo_inicial) {
    time_t tiempo_final = time(NULL);

    return tiempo_final - tiempo_inicial;
}

void decrementar_grado_multiprogramacion() {
    pthread_mutex_lock(&mutexGradoMultiprogramacion);
    GRADO_MULTIPROGRAMACION--;
    printf("GRADO_MULTIPROGRAMACION--: %d\n", GRADO_MULTIPROGRAMACION);
    pthread_mutex_unlock(&mutexGradoMultiprogramacion);
}

void incrementar_grado_multiprogramacion() {
    pthread_mutex_lock(&mutexGradoMultiprogramacion);
    GRADO_MULTIPROGRAMACION++;
    printf("GRADO_MULTIPROGRAMACION++: %d\n", GRADO_MULTIPROGRAMACION);
    pthread_mutex_unlock(&mutexGradoMultiprogramacion);
}

// si el proceso A es el seleccionado para ejecutar (ya que tiene menos ráfaga que el B y el C), 
// ya se sabe que entre el A, B y C siempre el A va a ser el que tenga mas prioridad. 
// hacer la comparación de ráfagas solo en el caso de que entre un nuevo proceso a ready.

void iniciar_algoritmo_planificacion(char* algoritmoPlanificacion, t_pcb* pcb) {
    strcmp("SJF", algoritmoPlanificacion) && list_size(READY) > 0 ?  
    agregar_proceso_y_replanificar_READY(pcb) : planificacion_FIFO(pcb);
}

void planificacion_FIFO(t_pcb* pcb) {
    decrementar_grado_multiprogramacion();
    enviarPCB(conexionCPUDispatch, pcb, PCB); 
    eliminar_proceso_de_READY(pcb);
}

void agregar_proceso_y_replanificar_READY(t_pcb* pcb) {
    list_add(READY, pcb);
    // cuando un proceso llegue a la cola READY se deberá enviar una Interrupción al proceso CPU
    enviar_interrupcion(conexionCPUInterrupt, DESALOJAR_PROCESO);
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
        if(tiempoBloqueado >= TIEMPO_MAXIMO_BLOQUEADO){
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

    incrementar_grado_multiprogramacion();
    printf("suspenderProceso() --> GRADO_MULTIPROGRAMACION--: %d\n", GRADO_MULTIPROGRAMACION);
    sem_post(&semGradoMultiprogramacion);
    // enviar mensaje a memoria con la información necesaria y se esperará la confirmación del mismo.

    //pasar a suspended ready: transición que se da al dormir ese proceso durante el tiempo restante 
    //entre el que se quiera bloquear y el maximo
}

void estimar_proxima_rafaga(time_t tiempo, t_pcb* pcb){
	int tiempo_cpu = tiempo / 1000;
	pcb->estimacionRafaga = ALFA*tiempo_cpu + (1-ALFA)*(pcb->estimacionRafaga);
}

void calcular_rafagas_restantes_proceso_desalojado(time_t tiempo_inicial, time_t tiempo_en_ejecucion, t_pcb* pcb_desalojada) {
    time_t rafagas_restantes = tiempo_inicial - tiempo_en_ejecucion;
    int tiempo_cpu = rafagas_restantes / 1000;
    pcb_desalojada->estimacionRafaga = tiempo_cpu;
}

void ordenar_procesos_lista_READY() {
    // Algoritmo SJF - ordenamiento
   list_sort(READY, sort_by_rafaga);

   printf("----- LISTA REORDENADA ----");
   for (int i=0; i < list_size(READY); i++){
        t_pcb* pcb = list_get(pcb, i);
        printf("pcb->idProceso->%d\testimacionRafaga-> %d\n",
               pcb->idProceso,
               pcb->estimacionRafaga);
    }
}

static bool sort_by_rafaga(void* pcb1, void* pcb2) {
    return (((t_pcb*) pcb1)->estimacionRafaga) < (((t_pcb*) pcb2)->estimacionRafaga);
}

void checkear_proceso_y_replanificar(t_pcb* pcbEnExec) {
    t_pcb* pcb = obtener_proceso_en_READY();

    if (pcb->estimacionRafaga < pcbEnExec->estimacionRafaga) { 
       replanificar_y_enviar_nuevo_proceso(pcb, pcbEnExec);
    }
}

void replanificar_y_enviar_nuevo_proceso(t_pcb* pcbNueva, t_pcb* pcbEnExec) {
    enviarPCB(conexionCPUDispatch, pcbNueva, PCB); 
    eliminar_proceso_de_READY(pcbNueva);
    agregar_proceso_y_replanificar_READY(pcbEnExec);
}
