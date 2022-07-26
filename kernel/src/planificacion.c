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

    iniciar_algoritmo_planificacion(pcbEnColaNew);
  
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
                    logear_PCB(logger,pcb,"RECIBIDO PARA BLOQUEAR");
                    if(list_size(READY)>0){
                    	t_pcb* pcb_ready = obtener_proceso_en_READY();
                    	logear_PCB(logger,pcb_ready,"PROCESO A EJECUTAR");
                    	enviarPCB(conexion_cpu_dispatch,pcb_ready,PCB);
                    	logear_PCB(logger,pcb_ready,"ENVIADO");
                    }
                    tiempo_en_ejecucion = calcular_tiempo_en_exec(tiempo_inicial);
                    estimar_proxima_rafaga(tiempo_en_ejecucion, pcb);
                    bloquearProceso(pcb);
                    t_pcb* pcb_desbloqueado = queue_pop(BLOCKED);
                    desbloquear_proceso(pcb_desbloqueado);
                    break;
                case TERMINAR_PROCESO:
                    log_info(logger, "TERMINANDO PROCESO");
                    t_pcb* pcbFinalizado = recibirPCB(conexion_cpu_dispatch);
                    logear_PCB(logger,pcbFinalizado,"RECIBIDO PARA TERMINAR");
                    avisarProcesoTerminado(pcbFinalizado->consola_fd);
                    incrementar_grado_multiprogramacion();
                    sem_post(&semGradoMultiprogramacion);
                    pcbFinalizado=NULL;
                    break;
                case DESALOJAR_PROCESO:
                	log_info(logger,"DESALOJANDO PROCESO");
                    t_pcb* pcb_desalojada = recibirPCB(conexion_cpu_dispatch);
                    logear_PCB(logger,pcb_desalojada,"RECIBIDO DESALOJADO");
                    tiempo_en_ejecucion = calcular_tiempo_en_exec(tiempo_inicial);
                    calcular_rafagas_restantes_proceso_desalojado(tiempo_en_ejecucion,pcb_desalojada);
                    checkear_proceso_y_replanificar(pcb_desalojada,CONTINUA_PROCESO);
                    break;
                default: ;
            }
        }
    }
    log_info(logger,"### FINALIZANDO CORTO PLAZO ###");
}

void eliminar_proceso_de_READY() {
    free(list_remove(READY, 0));
    log_info(logger,string_from_format("PROCESOS EN READY: [%d]",list_size(READY)));
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

void iniciar_algoritmo_planificacion(t_pcb* pcb) {
    crear_estructuras_memoria(pcb);
    seguir_algoritmo_planificacion(pcb,PCB);
}

void planificacion_FIFO(t_pcb* pcb) {
	log_info(logger,"ALGORITMO: FIFO");
    enviarPCB(conexion_cpu_dispatch, pcb, PCB);
    logear_PCB(logger,pcb,"ENVIADO");
    eliminar_proceso_de_READY(pcb);
}

bool altera_grado_multiprogramacion(op_code tipo_proceso){
	return (tipo_proceso != CONTINUA_PROCESO );
}

void agregar_proceso_READY(t_pcb* pcb, op_code tipo_proceso) {
	log_info(logger,"AGREGANDO PROCESO A READY...");
    list_add(READY, pcb);
    if(altera_grado_multiprogramacion(tipo_proceso)) decrementar_grado_multiprogramacion();
    log_info(logger,string_from_format("PROCESOS EN READY: [%d]",list_size(READY)));
}

bool interrupcion_por_proceso_en_ready(){
    if(hay_proceso_ejecutando()) {
    	printf("\n");
    	log_info(logger, "### ENVIANDO INTERRUPCION ###");
    	enviar_interrupcion(conexion_cpu_interrupt, DESALOJAR_PROCESO);
    	return true;
    }
    log_info(logger, "NO SE DEBE ENVIAR NINGUNA INTERRUPCION");
    return false;
}

void planificacion_SJF(t_pcb* pcb){
	log_info(logger,"ALGORITMO: SJF");
    bool desalojar_proceso_por_interrupcion = interrupcion_por_proceso_en_ready();
    if(!desalojar_proceso_por_interrupcion) ordenar_procesos_lista_READY();
}

void seguir_algoritmo_planificacion(t_pcb* pcb, op_code tipo_proceso){
	agregar_proceso_READY(pcb,tipo_proceso);
    (strcmp("SJF", ALGORITMO_PLANIFICACION)==0) ?
    planificacion_SJF(pcb) : planificacion_FIFO(pcb);
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

void desbloquear_proceso(t_pcb* pcb){
	seguir_algoritmo_planificacion(pcb,CONTINUA_PROCESO);
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
    agregar_proceso_READY(pcb_a_ready,PCB);
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
	log_info(logger,"ORDENANDO PROCESOS EN LISTA READY");
	if(list_size(READY)>1){
		list_sort(READY, sort_by_rafaga);
	}
	t_pcb* pcb_ready = obtener_proceso_en_READY();
	enviarPCB(conexion_cpu_dispatch,pcb_ready,PCB);
	logear_PCB(logger,pcb_ready,"ENVIADO A EJECUTAR");
	eliminar_proceso_de_READY();
	//decrementar_grado_multiprogramacion();
}

static bool sort_by_rafaga(void* pcb1, void* pcb2) {
    return (((t_pcb*) pcb1)->estimacionRafaga) < (((t_pcb*) pcb2)->estimacionRafaga);
}

void checkear_proceso_y_replanificar(t_pcb* pcbEnExec, op_code tipo_proceso) {
    t_pcb* pcb = obtener_proceso_en_READY();
    if (pcb->estimacionRafaga < pcbEnExec->estimacionRafaga) { 
       replanificar_y_enviar_nuevo_proceso(pcb, pcbEnExec, tipo_proceso);
    }else{
    	log_info(logger,"ENVIO PCB DE REGRESO");
    	enviarPCB(conexion_cpu_dispatch,pcbEnExec, PCB);
    	logear_PCB(logger,pcb,"ENVIADO");
    }
}

void replanificar_y_enviar_nuevo_proceso(t_pcb* pcbNueva, t_pcb* pcbEnExec, op_code tipo_proceso) {
	log_info(logger,"ENVIO PCB PROCESO MAS CORTO");
    enviarPCB(conexion_cpu_dispatch, pcbNueva, PCB);
    logear_PCB(logger,pcbNueva,"ENVIADO");
    eliminar_proceso_de_READY(pcbNueva);
    planificacion_SJF(pcbEnExec);
}

bool hay_proceso_ejecutando(){
	return (GRADO_MULTIPROGRAMACION < MAX_GRADO_MULTIPROGRAMACION-1);
}

/* ---------> MEMORIA <--------- */

void crear_estructuras_memoria(t_pcb* pcb) {
	printf("\n");
	log_info(logger,"### CREANDO ESTRUCTURAS DE MEMORIA###");
    int pcb_actualizado = 0;
    t_paquete* paquete = crearPaquete();
    enviarPCB(conexion_memoria, pcb, CREAR_ESTRUCTURAS_ADMIN );
    logear_PCB(logger,pcb,"ENVIADO A MEMORIA");
    while(conexion_memoria != -1 && pcb_actualizado != 0){
        op_code cod_op = recibirOperacion(conexion_memoria);
        switch(cod_op) {
            case ACTUALIZAR_INDICE_TABLA_PAGINAS:
                ;
                t_pcb* pcb_aux = recibirPCB(conexion_memoria);
                logear_PCB(logger,pcb,"RECIBIDO DE MEMORIA");
                pcb->tablaPaginas = pcb_aux->tablaPaginas;
                //printf("\nACTUALIZAR_TABLA_PAGINAS: PCB->idProceso: %ld, PCB->tablaPaginas:%ld\n", pcb->idProceso, pcb->tablaPaginas);
                pcb_actualizado=1;
                break;
        }
    }
}
