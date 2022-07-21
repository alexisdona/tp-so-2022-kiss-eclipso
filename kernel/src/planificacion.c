
#include <semaphore.h>
#include "include/planificacion.h"

double ALFA;

t_attrs_planificacion* attrs_plani;

void iniciarPlanificacion(t_attrs_planificacion* attrs_planificacion) {
	attrs_plani = attrs_planificacion;
    log_info(attrs_plani->logger, "Iniciando planificación...");
    pthread_mutex_lock(&mutexColaNew);
    queue_push(NEW, attrs_plani->pcb);
  //  printf("dentro de mutex de cola de new. Tamaño de la cola de NEW: %d\n", queue_size(NEW));
    pthread_mutex_unlock(&mutexColaNew);
    iniciarPlanificacionCortoPlazo(attrs_plani);
}

void iniciarPlanificacionCortoPlazo(t_attrs_planificacion* attrs_planificacion) {
    //  printf("Entra en inciarPlanificacion de corto plazo\n");
    tiempo_max_bloqueo = attrs_planificacion->tiempo_maximo_bloqueado;
    ALFA = attrs_planificacion->alpha;
    conexionCPUDispatch = attrs_planificacion->conexion_cpu_dispatch;
    conexionCPUInterrupt = attrs_planificacion->conexion_cpu_interrupt;

    sem_wait(&semGradoMultiprogramacion);
    pthread_mutex_lock(&mutexColaNew);
    t_pcb *pcbEnColaNew = queue_pop(NEW);
    pthread_mutex_unlock(&mutexColaNew);

    pthread_mutex_lock(&mutexColaReady);

    iniciar_algoritmo_planificacion(attrs_planificacion->algoritmo_planificacion, pcbEnColaNew);
  
    pthread_mutex_unlock(&mutexColaReady);

    time_t tiempo_inicial = time(NULL);

    printf("CPU-DISPATCH: %d\n",conexionCPUDispatch);

    while(conexionCPUDispatch != -1){
        op_code cod_op = recibirOperacion(conexionCPUDispatch);
        if(cod_op!=-1) {
        	printf("RECIBIENDO OPERACION\n");
        	printf("COD-OP: %d\n",cod_op);

            switch(cod_op){
                case BLOQUEAR_PROCESO:
                    printf("BLOQUEAR PROCESO.......\n");
                    t_pcb* pcb = recibirPCB(conexionCPUDispatch);
                    printf("PCB ID: %zu\n",pcb->idProceso);
                    printf("PCB PC: %zu\n",pcb->programCounter);
                    bloquearProceso(pcb);
                    tiempo_en_ejecucion = calcular_tiempo_en_exec(tiempo_inicial);
                    estimar_proxima_rafaga(tiempo_en_ejecucion, pcb);
                    break;
                case TERMINAR_PROCESO:
                    log_info(attrs_planificacion->logger, "PCB recibido para terminar proceso");
                    t_pcb* pcbFinalizado = recibirPCB(conexionCPUDispatch);
                    printf("pcbFinalizado->idProceso: %zu\n", pcbFinalizado->idProceso);
                    printf("pcbFinalizado->tamanioProceso: %zu\n", pcbFinalizado->tamanioProceso);
                    incrementar_grado_multiprogramacion();
                    printf("CONSOLA FD: %d\n",pcbFinalizado->consola_fd);
                    avisarProcesoTerminado(pcbFinalizado->consola_fd);
                    //enviarMensaje("Proceso terminado", pcbFinalizado->consola_fd);
                    sem_post(&semGradoMultiprogramacion);
                    //proceso_en_ejecucion=0;
                    //atendi_dispatch = 1;
                    break;
                case DESALOJAR_PROCESO:
                	printf("CPU-DISPATCH: %d\n",conexionCPUDispatch);
                	log_info(attrs_planificacion->logger, "PCB recibido desalojado por la CPU");
                    t_pcb* pcb_desalojada = recibirPCB(conexionCPUDispatch);
                    log_info(attrs_plani->logger,"PCB DESALOJADA RECIBIDA..");
                    printf("\n PID-PCB DESALOJADA: %d\n",pcb_desalojada->idProceso);

                    tiempo_en_ejecucion = calcular_tiempo_en_exec(tiempo_inicial);
                    printf("Tiempo en ejecucion: %d\n",tiempo_en_ejecucion);

                    calcular_rafagas_restantes_proceso_desalojado(tiempo_en_ejecucion,pcb_desalojada);
                    checkear_proceso_y_replanificar(pcb_desalojada);
                    break;
            }

        }

    }
}

void eliminar_proceso_de_READY() {

	printf("TAM-LISTA: %d\n",list_size(READY));
    free(list_remove(READY, 0));
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
	printf("%s\n",algoritmoPlanificacion);
    proceso_en_ready_memoria(pcb);
    (strcmp("SJF", algoritmoPlanificacion)==0) ?
    planificacion_SJF(pcb) : planificacion_FIFO(pcb);
}

void planificacion_FIFO(t_pcb* pcb) {
	log_info(attrs_plani->logger,"ALGORITMO: FIFO");
	agregar_proceso_READY(pcb);
    decrementar_grado_multiprogramacion();
    enviarPCB(conexionCPUDispatch, pcb, PCB);
    eliminar_proceso_de_READY(pcb);
}

void agregar_proceso_READY(t_pcb* pcb) {
	log_info(attrs_plani->logger,"AGREGANDO PROCESO A READY...");
    list_add(READY, pcb);
    printf("AGRUEGUE PROCESO A READY: %d\n",list_size(READY));
}

void interrupcion_por_proceso_en_ready(){
    if(list_size(READY)>0 && hay_proceso_en_ejecucion) {
    	log_info(attrs_plani->logger, "ENVIANDO INTERRUPCION");
    	enviar_interrupcion(conexionCPUInterrupt, DESALOJAR_PROCESO);
    }else{
    	hay_proceso_en_ejecucion=true;
    }
}

void planificacion_SJF(t_pcb* pcb){
	log_info(attrs_plani->logger,"ALGORITMO: SJF");
    agregar_proceso_READY(pcb);
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
            suspender_proceso(pcb);
        };
        if(tiempoBloqueado > TIEMPO_MAXIMO_BLOQUEADO){
            log_info(logger,"SUSPENDO EL PROCESO");
            suspender_proceso(pcb);
        } else {
        	//log_info(logger,string_from_format("BLOQUEO AL PROCESO POR %ds",(tiempoBloqueado/1000)));
        	usleep(tiempoBloqueado*1000);
        }
    }
}


void suspender_proceso(t_pcb* pcb) {
    pthread_mutex_lock(&mutexColaBloqueados);
    t_pcb * pcb_a_suspended_blocked = queue_pop(BLOCKED);
    pthread_mutex_unlock(&mutexColaBloqueados);

    pthread_mutex_lock(&mutexColaSuspendedBloqued); 
    enviarPCB(conexionMemoria,pcbEnColaBlocked,SWAPEAR_PROCESO);
    list_add(SUSPENDED_BLOCKED, pcb_a_suspended_blocked);
    pthread_mutex_unlock(&mutexColaSuspendedBloqued);
    
    incrementar_grado_multiprogramacion();
    printf("suspenderProceso() --> GRADO_MULTIPROGRAMACION--: %d\n", GRADO_MULTIPROGRAMACION);
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
	printf("INICIAL: %d\n",pcb_desalojada->estimacionRafaga);
	printf("EXEC: %d\n",tiempo_en_ejecucion);
	uint32_t rafagas_restantes = pcb_desalojada->estimacionRafaga - tiempo_en_ejecucion;
	printf("RESTANTES: %d\n",rafagas_restantes);
    pcb_desalojada->estimacionRafaga = rafagas_restantes;
}

void ordenar_procesos_lista_READY() {
	if(list_size(READY)>1){
		list_sort(READY, sort_by_rafaga);
	}
	enviarPCB(conexionCPUDispatch,obtener_proceso_en_READY(),PCB);

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
    	log_info(attrs_plani->logger,"ENVIO PCB DE REGRESO");
    	enviarPCB(conexionCPUDispatch,pcbEnExec, PCB);
    }
}

void replanificar_y_enviar_nuevo_proceso(t_pcb* pcbNueva, t_pcb* pcbEnExec) {
	log_info(attrs_plani->logger,"ENVIO PCB PROCESO MAS CORTO");
    enviarPCB(conexionCPUDispatch, pcbNueva, PCB); 
    eliminar_proceso_de_READY(pcbNueva);
    planificacion_SJF(pcbEnExec);
}

/* ---------> MEMORIA <--------- */

void proceso_en_ready_memoria(t_pcb* pcb ) {
    agregar_proceso_READY(pcb);
    crear_estructuras_memoria(pcbEnColaNew);
}

void crear_estructuras_memoria(t_pcb* pcb) {
    int pcb_actualizado = 0;
    t_paquete* paquete = crearPaquete();
    enviarPCB(conexionMemoria, pcb, CREAR_ESTRUCTURAS_ADMIN );
    printf("pcb->idProceso: %zu\n pcb->tamanioProceso:%zu", pcb->idProceso, pcb->tamanioProceso);
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
