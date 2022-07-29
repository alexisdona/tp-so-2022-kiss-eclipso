#include <semaphore.h>
#include "include/planificacion.h"

void iniciarPlanificacion(t_pcb* pcb){
	printf("\n");
    log_info(logger, "### INICIANDO PLANIFICACION ###");
    pthread_mutex_lock(&mutexColaNew);
    queue_push(NEW,pcb);
    log_info(logger,string_from_format("PCB ENCOLADO [%d] - TAMAÑO COLA NEW [%d]",pcb->idProceso,queue_size(NEW)));
    pthread_mutex_unlock(&mutexColaNew);
    iniciarPlanificacionCortoPlazo();
}

void iniciarPlanificacionCortoPlazo() {
	printf("\n");
	log_info(logger,"## INICIANDO CORTO PLAZO ##");
    sem_wait(&semGradoMultiprogramacion);
    pthread_mutex_lock(&mutexColaNew);
    t_pcb *pcbEnColaNew = queue_pop(NEW);
    log_info(logger,string_from_format("PCB DESENCOLADO [%d]- TAMAÑO COLA NEW [%d]",pcbEnColaNew->idProceso,queue_size(NEW)));
    pthread_mutex_unlock(&mutexColaNew);

    pthread_mutex_lock(&mutexColaReady);

    iniciar_algoritmo_planificacion(ALGORITMO_PLANIFICACION, pcbEnColaNew);
  
    pthread_mutex_unlock(&mutexColaReady);

    time_t tiempo_inicial = time(NULL);

    while(conexion_cpu_dispatch != -1){
    	op_code cod_op = recibirOperacion(conexion_cpu_dispatch);
        log_info(logger,string_from_format("OPERACION RECIBIDA DISPATCH [%d]",cod_op));
    	if(cod_op != -1) {
            switch(cod_op){
                case BLOQUEAR_PROCESO:
                    log_info(logger,"BLOQUEANDO PROCESO");
                    t_pcb* pcb = recibirPCB(conexion_cpu_dispatch);
                    logear_PCB(logger,pcb,"RECIBIDO");
                    bloquearProceso(pcb);
                    tiempo_en_ejecucion = calcular_tiempo_en_exec(tiempo_inicial);
                    estimar_proxima_rafaga(tiempo_en_ejecucion, pcb);
                    break;
                case TERMINAR_PROCESO:
                    log_info(logger, "TERMINANDO PROCESO");
                    t_pcb* pcbFinalizado = recibirPCB(conexion_cpu_dispatch);
                    logear_PCB(logger,pcbFinalizado,"RECIBIDO");
                    incrementar_grado_multiprogramacion();
                    avisarProcesoTerminado(pcbFinalizado->consola_fd);
                    sem_post(&semGradoMultiprogramacion);
                    break;
                case DESALOJAR_PROCESO:
                	log_info(logger,"DESALOJANDO PROCESO");
                    t_pcb* pcb_desalojada = recibirPCB(conexion_cpu_dispatch);
                    logear_PCB(logger,pcb_desalojada,"RECIBIDO");
                    tiempo_en_ejecucion = calcular_tiempo_en_exec(tiempo_inicial);
                    calcular_rafagas_restantes_proceso_desalojado(tiempo_en_ejecucion,pcb_desalojada);
                    checkear_proceso_y_replanificar(pcb_desalojada);
                    break;
            }
        }
    }
    log_info(logger,"### FINALIZANDO CORTO PLAZO ###");
}

void eliminar_proceso_de_READY() {
	//printf("TAM-LISTA: %d\n",list_size(READY));
    free(list_remove(READY, 0));
}

time_t calcular_tiempo_en_exec(time_t tiempo_inicial) {
    time_t tiempo_final = time(NULL);
    return tiempo_final - tiempo_inicial;
}

void decrementar_grado_multiprogramacion() {
    pthread_mutex_lock(&mutexGradoMultiprogramacion);
    GRADO_MULTIPROGRAMACION--;
    log_info(logger,string_from_format("GRADO MULTIPROGRAMACION [%d]",GRADO_MULTIPROGRAMACION));
    pthread_mutex_unlock(&mutexGradoMultiprogramacion);
}

void incrementar_grado_multiprogramacion() {
    pthread_mutex_lock(&mutexGradoMultiprogramacion);
    GRADO_MULTIPROGRAMACION++;
    log_info(logger,string_from_format("GRADO MULTIPROGRAMACION [%d]",GRADO_MULTIPROGRAMACION));
    pthread_mutex_unlock(&mutexGradoMultiprogramacion);
}

void iniciar_algoritmo_planificacion(char* algoritmoPlanificacion, t_pcb* pcb) {
    proceso_en_ready_memoria(pcb);
    (strcmp("SJF", algoritmoPlanificacion)==0) ?
    planificacion_SJF(pcb) : planificacion_FIFO(pcb);
}

void planificacion_FIFO(t_pcb* pcb) {
	log_info(logger,"ALGORITMO: FIFO");
	agregar_proceso_READY(pcb);
    decrementar_grado_multiprogramacion();
    enviarPCB(conexion_cpu_dispatch, pcb, PCB);
    logear_PCB(logger,pcb,"ENVIADO");
    eliminar_proceso_de_READY(pcb);
}

void agregar_proceso_READY(t_pcb* pcb) {
	log_info(logger,"AGREGANDO PROCESO A READY...");
    list_add(READY, pcb);
    log_info(logger,string_from_format("PROCESOS EN READY: [%d]",list_size(READY)));
}

void interrupcion_por_proceso_en_ready(){
    if(list_size(READY)>1 ) {
    	log_info(logger, "ENVIANDO INTERRUPCION");
    	enviar_interrupcion(conexion_cpu_interrupt, INTERRUPCION);
    }
}

void planificacion_SJF(t_pcb* pcb){
	log_info(logger,"ALGORITMO: SJF");
    interrupcion_por_proceso_en_ready();
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
        if(tiempoBloqueado > TIEMPO_MAXIMO_BLOQUEADO){
            log_info(logger,"SUSPENDO EL PROCESO");
            suspender_proceso(pcb);
        } else {
        	log_info(logger,string_from_format("BLOQUEO AL PROCESO POR %ds",(tiempoBloqueado/1000)));
        	usleep(tiempoBloqueado*1000);
        }
    }
}


void suspender_proceso(t_pcb* pcb) {
    pthread_mutex_lock(&mutexColaBloqueados);
    t_pcb * pcb_a_suspended_blocked = queue_pop(BLOCKED);
    pthread_mutex_unlock(&mutexColaBloqueados);

    pthread_mutex_lock(&mutexColaSuspendedBloqued); 
    enviarPCB(conexion_memoria,pcb_a_suspended_blocked,SWAPEAR_PROCESO);
    logear_PCB(logger,pcb,"ENVIADO");
    list_add(SUSPENDED_BLOCKED, pcb_a_suspended_blocked);
    pthread_mutex_unlock(&mutexColaSuspendedBloqued);
    
    incrementar_grado_multiprogramacion();
    //printf("suspenderProceso() --> GRADO_MULTIPROGRAMACION--: %d\n", GRADO_MULTIPROGRAMACION);
    sem_post(&semGradoMultiprogramacion);

    unsigned int tiempo_en_suspendido = tiempo_en_suspended_blocked(pcb_a_suspended_blocked);
    sleep(tiempo_en_suspendido);
    // enviar mensaje a memoria con la información necesaria y se esperará la confirmación del mismo.

    pthread_mutex_lock(&mutexColaSuspendedBloqued); 
    t_pcb* pcb_a_suspended_ready = list_get(SUSPENDED_BLOCKED, 0);
    list_remove(SUSPENDED_BLOCKED, 0);
    pthread_mutex_unlock(&mutexColaSuspendedBloqued);

    agregar_proceso_SUSPENDED_READY(pcb_a_suspended_ready);    
}

void agregar_proceso_SUSPENDED_READY(t_pcb* pcb) {

    pthread_mutex_lock(&mutex_cola_suspended_ready);
    list_add(SUSPENDED_READY, pcb);
    pthread_mutex_unlock(&mutex_cola_suspended_ready);

    enviar_proceso_SUSPENDED_READY_a_READY();
}

void enviar_proceso_SUSPENDED_READY_a_READY() {
    pthread_mutex_lock(&mutexColaReady);
    t_pcb* pcb_a_ready = list_get(SUSPENDED_READY, 0);
    list_remove(SUSPENDED_READY, 0);
    agregar_proceso_READY(pcb_a_ready);
    pthread_mutex_unlock(&mutexColaReady);
}

// diferencia entre el tiempo maximo y el tiempo de bloqueo para que un proceso en suspended-blocked pase a suspended-ready
unsigned int tiempo_en_suspended_blocked(t_pcb* pcb) {
    t_instruccion * instruccion = ((t_instruccion*) (list_get(pcb->listaInstrucciones,(pcb->programCounter)-1)));
    operando tiempo_bloqueado = instruccion->parametros[0];
    return tiempo_bloqueado - TIEMPO_MAXIMO_BLOQUEADO;
}

void estimar_proxima_rafaga(uint32_t tiempo, t_pcb* pcb){
	int tiempo_cpu = tiempo / 1000;
	pcb->estimacionRafaga = ALFA*tiempo_cpu + (1-ALFA)*(pcb->estimacionRafaga);
}

void calcular_rafagas_restantes_proceso_desalojado(uint32_t tiempo_en_ejecucion, t_pcb* pcb_desalojada) {
	tiempo_en_ejecucion = tiempo_en_ejecucion * 1000;
	//printf("INICIAL: %d\n",pcb_desalojada->estimacionRafaga);
	//printf("EXEC: %d\n",tiempo_en_ejecucion);
	uint32_t rafagas_restantes = pcb_desalojada->estimacionRafaga - tiempo_en_ejecucion;
	//printf("RESTANTES: %d\n",rafagas_restantes);
    pcb_desalojada->estimacionRafaga = rafagas_restantes;
}

void ordenar_procesos_lista_READY() {
	if(list_size(READY)>1){
		list_sort(READY, sort_by_rafaga);
	}
	t_pcb* pcb_ready = obtener_proceso_en_READY();
	enviarPCB(conexion_cpu_dispatch,pcb_ready,PCB);
	logear_PCB(logger,pcb_ready,"ENVIADO");
	eliminar_proceso_de_READY();
	decrementar_grado_multiprogramacion();
}

static bool sort_by_rafaga(void* pcb1, void* pcb2) {
    return (((t_pcb*) pcb1)->estimacionRafaga) < (((t_pcb*) pcb2)->estimacionRafaga);
}

void checkear_proceso_y_replanificar(t_pcb* pcbEnExec) {
    t_pcb* pcb = obtener_proceso_en_READY();
    if (pcb->estimacionRafaga < pcbEnExec->estimacionRafaga) { 
       replanificar_y_enviar_nuevo_proceso(pcb, pcbEnExec);
    }else{
    	log_info(logger,"ENVIO PCB DE REGRESO");
    	enviarPCB(conexion_cpu_dispatch,pcbEnExec, PCB);
    	logear_PCB(logger,pcb,"ENVIADO");
    }
}

void replanificar_y_enviar_nuevo_proceso(t_pcb* pcbNueva, t_pcb* pcbEnExec) {
	log_info(logger,"ENVIO PCB PROCESO MAS CORTO");
    enviarPCB(conexion_cpu_dispatch, pcbNueva, PCB);
    logear_PCB(logger,pcbNueva,"ENVIADO");
    eliminar_proceso_de_READY(pcbNueva);
    planificacion_SJF(pcbEnExec);
}

bool hay_proceso_ejecutando(){
	list_size(READY)+queue_size(BLOCKED);
}

/* ---------> MEMORIA <--------- */

void proceso_en_ready_memoria(t_pcb* pcb) {
    agregar_proceso_READY(pcb);
    crear_estructuras_memoria(pcb);
}

void crear_estructuras_memoria(t_pcb* pcb) {
    int pcb_actualizado = 0;
    t_paquete* paquete = crearPaquete();
    enviarPCB(conexion_memoria, pcb, CREAR_ESTRUCTURAS_ADMIN );
    logear_PCB(logger,pcb,"ENVIADO");
    while(conexion_memoria != -1 && pcb_actualizado == 0){
        op_code cod_op = recibirOperacion(conexion_memoria);
        switch(cod_op) {
            case ACTUALIZAR_INDICE_TABLA_PAGINAS:
                ;
                t_pcb* pcb_aux = recibirPCB(conexion_memoria);
                logear_PCB(logger,pcb,"RECIBIDO");
                pcb->tablaPaginas = pcb_aux->tablaPaginas;
                //printf("\nACTUALIZAR_TABLA_PAGINAS: PCB->idProceso: %ld, PCB->tablaPaginas:%ld\n", pcb->idProceso, pcb->tablaPaginas);
                pcb_actualizado=1;
                break;
        }
    }
}
