#include <semaphore.h>
#include "kernel.h"

//Variables globales
t_log* logger;
int kernel_fd;
t_queue * colaProcesosNew, *colaProcesosReady, *colaProcesosBlocked;
t_config * config;

uint32_t gradoMultiprogramacion;
sem_t semGradoMultiprogramacion;
pthread_mutex_t mutexColaNew;
pthread_mutex_t mutexColaReady;


void sighandler(int s) {
    cerrar_programa(logger);
    exit(0);
}

int main() {
    signal(SIGINT, sighandler);

   if (inicializarMutex() != 0){
        return EXIT_FAILURE;
    }

    logger = log_create("kernel.log", "KERNEL", 1, LOG_LEVEL_DEBUG);
    config = iniciarConfig(CONFIG_FILE);
    gradoMultiprogramacion = config_get_int_value(config,"GRADO_MULTIPROGRAMACION");
    sem_init(&semGradoMultiprogramacion,0, gradoMultiprogramacion);
    char* ipKernel= config_get_string_value(config,"IP_KERNEL");
    char* puertoKernel= config_get_string_value(config,"PUERTO_KERNEL");
    char* ipMemoria= config_get_string_value(config,"IP_MEMORIA");
    char* puertoMemoria= config_get_string_value(config,"PUERTO_MEMORIA");
    char* ipCpu= config_get_string_value(config,"IP_CPU");
    char* puertoCpu= config_get_string_value(config,"PUERTO_ESCUCHA");
//    int conexionMemoria = crearConexion(ipMemoria, puertoMemoria, "Kernel");
//    int conexionCPU = crearConexion(ipCpu, puertoCpu, "Kernel");
    kernel_fd = iniciarServidor(ipKernel, puertoKernel, logger);
	log_info(logger, "Kernel listo para recibir una consola");
    colaProcesosNew = queue_create();
    colaProcesosReady = queue_create();

    while (escuchar_consolas(logger, "KERNEL", kernel_fd));

    //cerrar_programa(logger);

    return 0;
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

void cerrar_programa(t_log* logger) {
	log_destroy(logger);
}

static void procesar_conexion(void* void_args) {
	t_procesar_conexion_attrs* attrs = (t_procesar_conexion_attrs*) void_args;
	t_log* logger = attrs->log;
    int consola_fd = attrs->fd;
    free(attrs);

    op_code cop;
    t_list* listaInstrucciones = list_create();

    while (consola_fd != -1) {

		op_code cod_op = recibirOperacion(consola_fd);
		switch (cod_op) {
			case MENSAJE:
                recibirMensaje(consola_fd, logger);
				break;
			case LISTA_INSTRUCCIONES:
			    listaInstrucciones = recibirListaInstrucciones(consola_fd);
                int tamanioProceso = recibirTamanioProceso(consola_fd);
                t_pcb* pcb = crearEstructuraPcb(listaInstrucciones, tamanioProceso);
           //     printf("\nPCB->idProceso:%d\n", pcb->idProceso);
            //    printf("\nPCB->tamanioProceso:%d\n", pcb->tamanioProceso);
            /*    for(uint32_t i=0; i<list_size(listaInstrucciones); i++){
                    t_instruccion *instruccion = list_get(pcb->listaInstrucciones,i);
                    printf("\nEN EL PCB\ninstruccion-->codigoInstruccion->%d\toperando1->%d\toperando2->%d\n",
                           instruccion->codigo_operacion,
                           instruccion->parametros[0],
                           instruccion->parametros[1]);
                }*/
               // printf("PCB->programCounter:%d", pcb->programCounter);
                pthread_mutex_lock(&mutexColaNew);
                queue_push(colaProcesosNew, pcb);
                printf("\n\ntamanio de la cola de procesos en new: %d\n\n", queue_size(colaProcesosNew));
                pthread_mutex_unlock(&mutexColaNew);

                sem_wait(&semGradoMultiprogramacion);
                pthread_mutex_lock(&mutexColaReady);
                queue_push(colaProcesosReady, queue_pop(colaProcesosNew));
                gradoMultiprogramacion--;
                printf("gradoMultiprogramacion: %d", gradoMultiprogramacion );
                pthread_mutex_unlock(&mutexColaReady);

           //     sem_post(&semGradoMultiprogramacion);
            //    printf("\n\ntamanio de la cola de procesos en ready: %d\n\n", queue_size(colaProcesosReady));
                enviarMensaje("Recibí la lista de instrucciones. Termino de ejecutar y te aviso", consola_fd);
                //si es fifo hago un pop de la cola de ready y envio ese pcb al CPU y hago signal al semaforo de grado de multiprogramacion
                break;
			case -1:
				log_info(logger, "La consola se desconecto.");
				consola_fd = -1;
				break;
			default:
				log_warning(logger,"Operacion desconocida.");
				break;
			}
	}
}

int recibir_opcion() {
	char* opcion = malloc(4);
	log_info(logger, "Ingrese una opcion: ");
	scanf("%s",opcion);
	int op = atoi(opcion);
	free(opcion);
	return op;
}

int validar_y_ejecutar_opcion_consola(int opcion, int consola_fd, int kernel_fd) {

	int continuar = 1;

	switch(opcion) {
		case 1:
			log_info(logger,"kernel continua corriendo, esperando nueva consola.");
			consola_fd = esperarConsola(kernel_fd);
			break;
		case 0:
			log_info(logger,"Terminando kernel...");
			close(kernel_fd);
			continuar = 0;
			break;
		default:
			log_error(logger,"Opcion invalida. Volve a intentarlo");
			recibir_opcion();
	}

	return continuar;

}


int accion_kernel(int consola_fd, int kernel_fd) {

	log_info(logger, "¿Desea mantener el kernel corriendo? 1- Si 0- No");
	int opcion = recibir_opcion();

	if (recibirOperacion(consola_fd) == SIN_CONSOLAS) {
	  log_info(logger, "No hay mas clientes conectados.");
	  log_info(logger, "¿Desea mantener el kernel corriendo? 1- Si 0- No");
	  opcion = recibir_opcion();
	  return validar_y_ejecutar_opcion_consola(opcion, consola_fd, kernel_fd);
	}

	return validar_y_ejecutar_opcion_consola(opcion, consola_fd, kernel_fd);

}

int escuchar_consolas(t_log* logger, char* nombre_kernel, int kernel_fd) {
    int consola_fd = esperarConsola(kernel_fd);
    if (consola_fd != -1) {
        pthread_t hilo;
        t_procesar_conexion_attrs* attrs = malloc(sizeof(t_procesar_conexion_attrs));
        attrs->log = logger;
        attrs->fd = consola_fd;
        attrs->nombre_kernel = nombre_kernel;
        pthread_create(&hilo, NULL, (void*) procesar_conexion, (void*) attrs);
        pthread_detach(hilo);
        return 1;
    }
    return 0;
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}

t_pcb* crearEstructuraPcb(t_list* listaInstrucciones, int tamanioProceso) {

    t_pcb *pcb =  malloc(sizeof(t_pcb));
    t_instruccion *instruccion = list_get(listaInstrucciones,0);

    pcb->idProceso = process_get_thread_id();
    pcb->tamanioProceso = tamanioProceso;
    pcb->listaInstrucciones = listaInstrucciones;
    pcb->programCounter= instruccion->codigo_operacion;
    pcb->estimacionRafaga =1; // por ahora dejamos 1 como valor

}


