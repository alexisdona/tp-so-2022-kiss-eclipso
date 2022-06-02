#include <semaphore.h>
#include <include/planificador.h>
#include "kernel.h"

//Variables globales
t_log* logger;
int kernel_fd, conexionCPUDispatch;
t_config * config;

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

    logger = iniciarLogger("kernel.log", "KERNEL");
    config = iniciarConfig(CONFIG_FILE);
    GRADO_MULTIPROGRAMACION = config_get_int_value(config,"GRADO_MULTIPROGRAMACION");
    sem_init(&semGradoMultiprogramacion,0, GRADO_MULTIPROGRAMACION);
    char* ipKernel= config_get_string_value(config,"IP_KERNEL");
    char* puertoKernel= config_get_string_value(config,"PUERTO_ESCUCHA");
    char* ipMemoria= config_get_string_value(config,"IP_MEMORIA");
    int puertoMemoria= config_get_int_value(config,"PUERTO_MEMORIA");
    char* ipCpu= config_get_string_value(config,"IP_CPU");
    int puertoCpuDispatch= config_get_int_value(config,"PUERTO_CPU_DISPATCH");
    char* puertoCpuInterrupt = config_get_string_value(config,"PUERTO_CPU_INTERRUPT");
 //   int conexionMemoria = crearConexion(ipMemoria, puertoMemoria, "Kernel");
    conexionCPUDispatch = crearConexion(ipCpu, puertoCpuDispatch, "Kernel");
    kernel_fd = iniciarServidor(ipKernel, puertoKernel, logger);
	log_info(logger, "Kernel listo para recibir una consola");
    NEW = queue_create();
    READY = queue_create();

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
                enviarMensaje("Recibí la lista de instrucciones. Termino de ejecutar y te aviso", consola_fd);
			    listaInstrucciones = recibirListaInstrucciones(consola_fd);
                int tamanioProceso = recibirTamanioProceso(consola_fd);
                t_pcb* pcb = crearEstructuraPcb(listaInstrucciones, tamanioProceso);

                pthread_mutex_lock(&mutexColaNew);
                queue_push(NEW, pcb);
                printf("\n\ntamanio de la cola de procesos en new esperando grado de multiprogramación: %d\n\n", queue_size(NEW));
                pthread_mutex_unlock(&mutexColaNew);

                sem_wait(&semGradoMultiprogramacion);
                pthread_mutex_lock(&mutexColaReady);
                queue_push(READY, queue_pop(NEW));
                GRADO_MULTIPROGRAMACION--;

                printf("gradoMultiprogramacion: %d", GRADO_MULTIPROGRAMACION );
                pthread_mutex_unlock(&mutexColaReady);
                enviarPCB(conexionCPUDispatch, queue_pop(READY) );
           //     sem_post(&semGradoMultiprogramacion);
            //    printf("\n\ntamanio de la cola de procesos en ready: %d\n\n", queue_size(colaProcesosReady));

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
			consola_fd = esperarCliente(kernel_fd, logger);
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
    int consola_fd = esperarCliente(kernel_fd, logger);
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
    pcb->programCounter= instruccion->codigo_operacion;
    pcb->estimacionRafaga =1; // por ahora dejamos 1 como valor
    pcb->duracionUltimaRafaga =0; //Arranca en cero
    pcb->listaInstrucciones = listaInstrucciones;

    return pcb;
}




