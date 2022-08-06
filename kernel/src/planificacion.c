#include <semaphore.h>
#include "include/planificacion.h"

void iniciarPlanificacion(t_pcb* pcb){
	printf(GRN"\n");
    log_info(logger, "### INICIANDO PLANIFICACION ###");
    pthread_mutex_lock(&mutexColaNew);
    queue_push(NEW,pcb);
    log_info(logger,string_from_format("ENCOLADO [%d] - NEW [%d]",pcb->idProceso,queue_size(NEW)));
    pthread_mutex_unlock(&mutexColaNew);
    iniciarPlanificacionCortoPlazo();
}

void iniciarPlanificacionCortoPlazo(){
	printf(GRN"\n");
	log_info(logger,"## INICIANDO CORTO PLAZO ##");
    sem_wait(&semGradoMultiprogramacion);
    t_pcb* pcb_a_ready = obtener_PCB_segun_prioridad();

    pthread_mutex_lock(&mutexColaReady);
    iniciar_algoritmo_planificacion(pcb_a_ready);
    pthread_mutex_unlock(&mutexColaReady);

    if(hay_proceso_ejecutando()) tiempo_inicial = time(NULL);

    while(conexion_cpu_dispatch != -1){
    	pthread_mutex_lock(&mutex_dispatch);
    	op_code cod_op = recibirOperacion(conexion_cpu_dispatch);
    	if(cod_op != -1) {
            switch(cod_op){
				case TERMINAR_PROCESO:
					printf(GRN"\n");
					log_info(logger, "# TERMINANDO PROCESO #");
					t_pcb* pcbFinalizado = recibirPCB(conexion_cpu_dispatch);
					pthread_mutex_unlock(&mutex_dispatch);
					proceso_ejecutando=0;
					logear_PCB(logger,pcbFinalizado,"RECIBIDO PARA TERMINAR");
					pthread_mutex_lock(&mutex_memoria);
                    enviarPCB(conexion_memoria, pcbFinalizado, TERMINAR_PROCESO);
                    pthread_mutex_unlock(&mutex_memoria);
					avisarProcesoTerminado(pcbFinalizado->consola_fd);
					incrementar_grado_multiprogramacion();
					continuar_planificacion();
					break;
                case BLOQUEAR_PROCESO:
                	printf(GRN"\n");
                	log_info(logger,"# BLOQUEANDO PROCESO #");
                    t_pcb* pcb = recibirPCB(conexion_cpu_dispatch);
                    pthread_mutex_unlock(&mutex_dispatch);
                    logear_PCB(logger,pcb,"RECIBIDO PARA BLOQUEAR");
                    proceso_ejecutando=0;
                    if(list_size(READY)>0){
                    	t_pcb* pcb_ready = obtener_proceso_en_READY();
                    	logear_PCB(logger,pcb_ready,"PROCESO A EJECUTAR");
                    	enviarPCB(conexion_cpu_dispatch,pcb_ready,PCB);
                    	eliminar_proceso_de_READY();
                    	proceso_ejecutando=1;
                    }
                    tiempo_en_ejecucion = calcular_tiempo_en_exec(tiempo_inicial);
                    estimar_proxima_rafaga(tiempo_en_ejecucion, pcb);
                    bloquearProceso(pcb);
                    break;
                case DESALOJAR_PROCESO:
                	printf(GRN"\n");
                	log_info(logger,"# DESALOJANDO PROCESO #");
                    t_pcb* pcb_desalojada = recibirPCB(conexion_cpu_dispatch);
                    pthread_mutex_unlock(&mutex_dispatch);
                    proceso_ejecutando=0;
                    logear_PCB(logger,pcb_desalojada,"RECIBIDO DESALOJADO");
                    tiempo_en_ejecucion = calcular_tiempo_en_exec(tiempo_inicial);
                    calcular_rafagas_restantes_proceso_desalojado(tiempo_en_ejecucion,pcb_desalojada);
                    checkear_proceso_y_replanificar(pcb_desalojada);
                    break;
    			default:
    				printf("cod_op: [%d]\tconexion_cpu_dispatch: [%d]\n",cod_op,conexion_cpu_dispatch);
    				log_warning(logger,string_from_format("OPERACION DESCONOCIDA DISPATCH: COD-OP [%d]",cod_op));
    				break;
            }
        }
    }
    log_info(logger,"### FINALIZANDO CORTO PLAZO ###");
}

t_pcb* obtener_PCB_segun_prioridad(){
	t_pcb* pcb_a_ready;
	if(queue_size(SUSPENDED_READY) > 0){
	    pthread_mutex_lock(&mutex_cola_suspended_ready);
	    pcb_a_ready = queue_pop(SUSPENDED_READY);
	    log_info(logger,string_from_format("DESENCOLADO [%d] - S.Ready [%d]",pcb_a_ready->idProceso,queue_size(SUSPENDED_READY)));
	    pthread_mutex_unlock(&mutex_cola_suspended_ready);
	}
	else if(queue_size(NEW) > 0){
	    pthread_mutex_lock(&mutexColaNew);
	    pcb_a_ready = queue_pop(NEW);
	    log_info(logger,string_from_format("DESENCOLADO [%d] - NEW [%d]",pcb_a_ready->idProceso,queue_size(NEW)));
	    pthread_mutex_unlock(&mutexColaNew);
	}else{
		pcb_a_ready = NULL;
	}
	return pcb_a_ready;
}

void eliminar_proceso_de_READY() {
    free(list_remove(READY, 0));
    log_info(logger,string_from_format("READY-1: [%d]",list_size(READY)));
}

time_t calcular_tiempo_en_exec(time_t tiempo_inicial) {
    time_t tiempo_final = time(NULL);
    return tiempo_final - tiempo_inicial;
}

void decrementar_grado_multiprogramacion() {
    pthread_mutex_lock(&mutexGradoMultiprogramacion);
    GRADO_MULTIPROGRAMACION--;
    log_info(logger,string_from_format("GRADO MULTIPROGRAMACION-1 [%d]",GRADO_MULTIPROGRAMACION));
    pthread_mutex_unlock(&mutexGradoMultiprogramacion);
}

void incrementar_grado_multiprogramacion() {
    pthread_mutex_lock(&mutexGradoMultiprogramacion);
    GRADO_MULTIPROGRAMACION++;
    log_info(logger,string_from_format("GRADO MULTIPROGRAMACION+1 [%d]",GRADO_MULTIPROGRAMACION));
    sem_post(&semGradoMultiprogramacion);
    pthread_mutex_unlock(&mutexGradoMultiprogramacion);
}

void iniciar_algoritmo_planificacion(t_pcb* pcb) {
    crear_estructuras_memoria(pcb);
    seguir_algoritmo_planificacion(pcb,PCB);
}

void planificacion_FIFO(t_pcb* pcb) {
	log_info(logger,"ALGORITMO: FIFO");
	if(!hay_proceso_ejecutando() && (hay_procesos_pendientes()>0)){
		enviarPCB(conexion_cpu_dispatch, pcb, PCB);
		proceso_ejecutando=1;
		logear_PCB(logger,pcb,"ENVIADO POR FIFO");
	    eliminar_proceso_de_READY();
	}
}

bool altera_grado_multiprogramacion(op_code tipo_proceso){
	return (tipo_proceso != CONTINUA_PROCESO );
}

void agregar_proceso_READY(t_pcb* pcb, op_code tipo_proceso) {
	log_info(logger,"AGREGANDO PROCESO A READY...");
    list_add(READY, pcb);
    if(altera_grado_multiprogramacion(tipo_proceso)) decrementar_grado_multiprogramacion();
    log_info(logger,string_from_format("READY+1: [%d]",list_size(READY)));
}

bool interrupcion_por_proceso_en_ready(){
    if(hay_proceso_ejecutando()) {
    	printf(GRN"\n");
    	log_info(logger, "### ENVIANDO INTERRUPCION ###");
    	enviar_interrupcion(conexion_cpu_interrupt, INTERRUPCION);
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

void continuar_planificacion(){
	if(list_size(READY)>0){
		log_info(logger,"CONTINUANDO PLANIFICACION, AUN HAY PROCESOS POR EJECUTAR");
	    (strcmp("SJF", ALGORITMO_PLANIFICACION)==0) ?
	    planificacion_SJF(obtener_proceso_en_READY()) : planificacion_FIFO(obtener_proceso_en_READY());
	}else{
		if(GRADO_MULTIPROGRAMACION>0){
			t_pcb* pcb_a_ready = obtener_PCB_segun_prioridad();
			if(pcb_a_ready!=NULL) {
                agregar_proceso_READY(pcb_a_ready,SUSPENDER_PROCESO);
				continuar_planificacion();
			}
		}
	}
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

    if(pthread_mutex_init(&mutex_dispatch, NULL) != 0){
    	perror("Mutex conexion dispatch: ");
    	error+=1;
    }

    if(pthread_mutex_init(&mutex_memoria, NULL) != 0){
    	perror("Mutex conexion dispatch: ");
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
    operando tiempoBloqueado = obtener_tiempo_para_bloquear(pcb);
	if(tiempoBloqueado > TIEMPO_MAXIMO_BLOQUEADO){
		log_info(logger,"SUSPENDO EL PROCESO");
		suspender_proceso(pcb);
	} else {
		pthread_mutex_lock(&mutexColaBloqueados);
		queue_push(BLOCKED, pcb);
		pthread_mutex_unlock(&mutexColaBloqueados);

		pthread_mutex_lock(&mutexColaBloqueados);
		t_pcb* pcb_bloquear = queue_pop(BLOCKED);
		pthread_mutex_unlock(&mutexColaBloqueados);
		tiempoBloqueado = obtener_tiempo_para_bloquear(pcb_bloquear);
		log_info(logger,string_from_format("BLOQUEO AL PROCESO POR %ds",(tiempoBloqueado/1000)));
		usleep(tiempoBloqueado*1000);
        desbloquear_proceso(pcb_bloquear);
	}
}

uint32_t obtener_tiempo_para_bloquear(t_pcb* pcb){
	t_instruccion * instruccion = ((t_instruccion*) (list_get(pcb->listaInstrucciones,(pcb->programCounter)-1)));
	return (instruccion->parametros[0]);
}

void desbloquear_proceso(t_pcb* pcb){
	log_info(logger,"PROCESO DESBLOQUEADO");
	seguir_algoritmo_planificacion(pcb,CONTINUA_PROCESO);
}

void suspender_proceso(t_pcb* pcb) {
    pthread_mutex_lock(&mutexColaSuspendedBloqued);
    queue_push(SUSPENDED_BLOCKED,pcb);
    pthread_mutex_unlock(&mutexColaSuspendedBloqued);

    pthread_mutex_lock(&mutexColaSuspendedBloqued);
    t_pcb* pcb_suspender = queue_pop(SUSPENDED_BLOCKED);
    pthread_mutex_unlock(&mutexColaSuspendedBloqued);

    enviarPCB(conexion_memoria,pcb_suspender,SWAPEAR_PROCESO);
    logear_PCB(logger,pcb_suspender,"ENVIADO SWAPEAR");
    
    incrementar_grado_multiprogramacion();

    unsigned int tiempo_en_suspendido = tiempo_en_suspended_blocked(pcb_suspender);
    printf("TIEMPO A SUSPENDER: %d\n",tiempo_en_suspendido);
    usleep(tiempo_en_suspendido*1000);
    agregar_proceso_SUSPENDED_READY(pcb_suspender);
}

void agregar_proceso_SUSPENDED_READY(t_pcb* pcb) {
    pthread_mutex_lock(&mutex_cola_suspended_ready);
    queue_push(SUSPENDED_READY, pcb);
    pthread_mutex_unlock(&mutex_cola_suspended_ready);
    log_info(logger,"PROCESO EN S.READY");
    continuar_planificacion();
}

unsigned int tiempo_en_suspended_blocked(t_pcb* pcb) {
    operando tiempo_bloqueado = obtener_tiempo_para_bloquear(pcb);
    return tiempo_bloqueado - TIEMPO_MAXIMO_BLOQUEADO;
}

void estimar_proxima_rafaga(uint32_t tiempo, t_pcb* pcb){
	int tiempo_cpu = tiempo / 1000;
	pcb->estimacionRafaga = ALFA*tiempo_cpu + (1-ALFA)*(pcb->estimacionRafaga);
}

void calcular_rafagas_restantes_proceso_desalojado(uint32_t tiempo_en_ejecucion, t_pcb* pcb_desalojada) {
	tiempo_en_ejecucion = tiempo_en_ejecucion * 1000;
	uint32_t rafagas_restantes = pcb_desalojada->estimacionRafaga - tiempo_en_ejecucion;
    pcb_desalojada->estimacionRafaga = rafagas_restantes;
}

void ordenar_procesos_lista_READY() {
	log_info(logger,"ORDENANDO PROCESOS EN LISTA READY");

	if(list_size(READY)>1){
        for(int i=0;i< list_size(READY);i++){
            t_pcb* pcbloco = list_get(READY, i);
            printf("pid %d estimado %d", pcbloco->idProceso, pcbloco->estimacionRafaga);
        }
		list_sort(READY, sort_by_rafaga);
        for(int i=0;i< list_size(READY);i++){
            t_pcb* pcbloco = list_get(READY, i);
            printf("pid %d estimado %d", pcbloco->idProceso, pcbloco->estimacionRafaga);
        }
	}

	t_pcb* pcb_ready = obtener_proceso_en_READY();
	enviarPCB(conexion_cpu_dispatch,pcb_ready,PCB);
	proceso_ejecutando=1;
	logear_PCB(logger,pcb_ready,"ENVIADO A EJECUTAR");
	eliminar_proceso_de_READY();
}

static bool sort_by_rafaga(void* pcb1, void* pcb2) {
	t_pcb* pcb_a = (t_pcb*) pcb1;
	t_pcb* pcb_b = (t_pcb*) pcb2;
    bool igual_estimacion = pcb_a->estimacionRafaga == pcb_b->estimacionRafaga;
    if(igual_estimacion) return pcb_a->idProceso < pcb_b->idProceso;
    else return pcb_a->estimacionRafaga < pcb_b->estimacionRafaga;
}

void checkear_proceso_y_replanificar(t_pcb* pcbEnExec) {
	agregar_proceso_READY(pcbEnExec,CONTINUA_PROCESO);
	ordenar_procesos_lista_READY();
}

void replanificar_y_enviar_nuevo_proceso(t_pcb* pcbNueva, t_pcb* pcbEnExec) {
	log_info(logger,"ENVIO PCB PROCESO MAS CORTO");
    enviarPCB(conexion_cpu_dispatch, pcbNueva, PCB);
    proceso_ejecutando=1;
    logear_PCB(logger,pcbNueva,"ENVIADO A EJECUTAR");
    eliminar_proceso_de_READY();
    planificacion_SJF(pcbEnExec);
}

bool hay_proceso_ejecutando(){
	return (proceso_ejecutando!=0);
}

void enviar_interrupcion(int socket, op_code cod_op) {
    t_paquete* paquete = crearPaquete();
    paquete->codigo_operacion = cod_op;
    enviarPaquete(paquete, socket);
    eliminarPaquete(paquete);
}

uint32_t hay_procesos_pendientes(){
	uint32_t procesos_pendientes=0;
	procesos_pendientes+=list_size(READY)+queue_size(NEW)+queue_size(BLOCKED)+queue_size(SUSPENDED_BLOCKED)+queue_size(SUSPENDED_READY);
	return procesos_pendientes;
}

/* ---------> MEMORIA <--------- */

void crear_estructuras_memoria(t_pcb* pcb) {
	printf(GRN"\n");
	log_info(logger,"### CREANDO ESTRUCTURAS DE MEMORIA###");
    int pcb_actualizado = 0;
    enviarPCB(conexion_memoria, pcb, CREAR_ESTRUCTURAS_ADMIN );
    logear_PCB(logger,pcb,"ENVIADO A MEMORIA");
    while(conexion_memoria != -1 && pcb_actualizado == 0) {
        op_code cod_op = recibirOperacion(conexion_memoria);
        switch(cod_op) {
            case ACTUALIZAR_INDICE_TABLA_PAGINAS:
                ;
                t_pcb* pcb_aux = recibirPCB(conexion_memoria);
                logear_PCB(logger,pcb,"RECIBIDO DE MEMORIA");
                pcb->tablaPaginas = pcb_aux->tablaPaginas;
                pcb_actualizado=1;
                break;
            default: ;
        }
    }
}
