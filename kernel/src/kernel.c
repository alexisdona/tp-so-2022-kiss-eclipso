#include <semaphore.h>
#include "include/planificacion.h"
#include "kernel.h"

//Variables globales

t_log* logger;
t_config * config;
t_attrs_planificacion* info_planificacion;

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
    TIEMPO_MAXIMO_BLOQUEADO = config_get_int_value(config, "TIEMPO_MAXIMO_BLOQUEADO");
    sem_init(&semGradoMultiprogramacion, 0, GRADO_MULTIPROGRAMACION);

    char* IP_KERNEL = config_get_string_value(config,"IP_KERNEL");
    char* PUERTO_KERNEL= config_get_string_value(config,"PUERTO_ESCUCHA");
    char* IP_MEMORIA= config_get_string_value(config,"IP_MEMORIA");
    int PUERTO_MEMORIA = config_get_int_value(config,"PUERTO_MEMORIA");
    char* IP_CPU = config_get_string_value(config,"IP_CPU");
    char* ALGORITMO_PLANIFICACION = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    int PUERTO_CPU_DISPATCH = config_get_int_value(config,"PUERTO_CPU_DISPATCH");
    char* PUERTO_CPU_INTERRUPT = config_get_string_value(config,"PUERTO_CPU_INTERRUPT");
    double ALFA = config_get_double_value(config, "ALFA");

    /* CONEXIONES A MODULOS */

    conexionMemoria = crearConexion(IP_MEMORIA, PUERTO_MEMORIA, "Kernel");
    conexionCPUDispatch = crearConexion(IP_CPU, PUERTO_CPU_DISPATCH, "Kernel");
    conexionCPUInterrupt = crearConexion(IP_CPU, PUERTO_CPU_INTERRUPT, "Kernel");

    enviarMensaje("hola CPU soy el kernel", conexionCPUDispatch);
    enviarMensaje("hola  MEMORIA soy el kernel", conexionMemoria);

    kernel_fd = iniciarServidor(IP_KERNEL, PUERTO_KERNEL, logger);
	log_info(logger, "Kernel listo para recibir una consola");

    /* INICIALIZACION DE COLAS Y LISTAS */

    NEW = queue_create();
    READY = list_create();
    BLOCKED = queue_create();
    SUSPENDED_BLOCKED = queue_create();

    /* Atributos a enviar para la planificacion */

    t_attrs_planificacion* attrs_planificacion = malloc(sizeof(t_attrs_planificacion));

    attrs_planificacion->conexion_cpu_dispatch = conexionCPUDispatch;
    attrs_planificacion->conexion_cpu_interrupt = conexionCPUInterrupt;
    attrs_planificacion->algoritmo_planificacion = ALGORITMO_PLANIFICACION;
    attrs_planificacion->logger = logger;
    attrs_planificacion->tiempo_maximo_bloqueado = TIEMPO_MAXIMO_BLOQUEADO;
    attrs_planificacion->alpha = ALFA;

    info_planificacion = attrs_planificacion;

    while (1) {
        escucharClientes("KERNEL");
    }
    //cerrar_programa(logger);
    return 0;
}


void cerrar_programa(t_log* logger) {
	log_destroy(logger);
}

void procesar_conexion(void* void_args) {
	t_procesar_conexion_attrs* attrs = (t_procesar_conexion_attrs*) void_args;
	t_log* logger = attrs->log;
    int cliente_fd = attrs->fd;
    free(attrs);

    while (cliente_fd != -1) {
        printf("entro un cliente nuevo: %d\n", cliente_fd);
		op_code cod_op = recibirOperacion(cliente_fd);
       
		switch (cod_op) {
			case MENSAJE:
                recibirMensaje(cliente_fd, logger);
				break;
			case LISTA_INSTRUCCIONES:
                enviarMensaje("Recibí la lista de instrucciones. Termino de ejecutar y te aviso", cliente_fd);
			    t_list* listaInstrucciones = recibirListaInstrucciones(cliente_fd);
                int tamanioProceso = recibirTamanioProceso(cliente_fd);
                t_pcb* pcb = crearEstructuraPcb(listaInstrucciones, tamanioProceso, cliente_fd);
                info_planificacion->pcb = pcb;
                //  printf("pcb->idProceso: %zu\n",pcb->idProceso);
                iniciarPlanificacion(info_planificacion);
                break;
        
            case -1:
				log_info(logger, "La consola se desconecto.");
              //  cliente_fd = -1;
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

int escucharClientes(char *nombre_kernel) {
    int cliente = esperarCliente(kernel_fd, logger);
    if (cliente != -1) {
        pthread_t hilo;
        t_procesar_conexion_attrs* attrs = malloc(sizeof(t_procesar_conexion_attrs));
        attrs->log = logger;
        attrs->fd = cliente;
        attrs->nombre_kernel = nombre_kernel;
        pthread_create(&hilo, NULL, (void*) procesar_conexion, (void*) attrs);
        pthread_detach(hilo);
        return 1;
    }
    return 0;
}

t_pcb* crearEstructuraPcb(t_list* listaInstrucciones, int tamanioProceso, int socketConsola) {

    t_pcb *pcb =  malloc(sizeof(t_pcb));
    t_instruccion *instruccion = list_get(listaInstrucciones,0);
    int estimacionInicial = config_get_int_value(config,"ESTIMACION_INICIAL");

    pcb->idProceso = process_get_thread_id();
    pcb->tamanioProceso = tamanioProceso;
    pcb->listaInstrucciones = listaInstrucciones;
    pcb->programCounter= instruccion->codigo_operacion;
    pcb->estimacionRafaga = estimacionInicial; 
    pcb->consola_fd = socketConsola;
    pcb->kernel_fd = kernel_fd;
    pcb->listaInstrucciones = listaInstrucciones;

    return pcb;
}



