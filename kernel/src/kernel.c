
#include "kernel.h"

t_log* logger;
int kernel_fd;
<<<<<<< HEAD
=======
t_queue * colaProcesosNew;
t_config * config;
>>>>>>> desarrollo

void sighandler(int s) {
    cerrar_programa(logger);
    exit(0);
}

int main() {
    signal(SIGINT, sighandler);

    logger = log_create("kernel.log", "KERNEL", 1, LOG_LEVEL_DEBUG);
<<<<<<< HEAD

    kernel_fd = iniciar_kernel();
    log_info(logger, "Kernel listo para recibir una consola");
=======
    config = iniciar_config();

    char* ip= config_get_string_value(config,"IP_MEMORIA");
    char* puerto= config_get_string_value(config,"PUERTO_MEMORIA");

    kernel_fd = iniciar_kernel(ip,puerto);

    ip= config_get_string_value(config,"IP_CPU");
    puerto= config_get_string_value(config,"PUERTO_ESCUCHA");
    kernel_fd = iniciar_kernel(ip,puerto);

	log_info(logger, "Kernel listo para recibir una consola");
    colaProcesosNew = queue_create();
>>>>>>> desarrollo

    while (escuchar_consolas(logger, "KERNEL", kernel_fd));

    //cerrar_programa(logger);

    return 0;
}

void cerrar_programa(t_log* logger) {
    log_destroy(logger);
}

static void procesar_conexion(void* void_args) {
    t_procesar_conexion_attrs* attrs = (t_procesar_conexion_attrs*) void_args;
    t_log* logger = attrs->log;
    int consola_fd = attrs->fd;
    //char* nombre_kernel = attrs->nombre_kernel;
    free(attrs);

    op_code cop;

    while (consola_fd != -1) { 
        
       if (recv(consola_fd, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
            log_info(logger, "CONSOLA DESCONECTADA");
            return;
        }

        op_code cod_op = recibirOperacion(consola_fd);

        switch (cod_op) {
            case MENSAJE:
                recibirMensaje(consola_fd);
                break;
            case LISTA_INSTRUCCIONES:
                printf("va a entrar en recibirListaInstrucciones\n");
                t_list* listaInstrucciones = list_create();

                listaInstrucciones = recibirListaInstrucciones(consola_fd);
                for(uint32_t i=0; i<list_size(listaInstrucciones); i++){
                    t_instruccion *instruccion = list_get(listaInstrucciones,i);
                    printf("\ninstruccion-->codigoInstruccion->%d\toperando1->%d\toperando2->%d\n",
                           instruccion->codigo_operacion,
                           instruccion->parametros[0],
                           instruccion->parametros[1]);
                }

                int tamanioProceso = recibirTamanioProceso(consola_fd);
                printf("\nTamano del proceso -->: %d", tamanioProceso);
                /*log_info(logger, "Me llegaron los siguientes valores:\n");
                list_iterate(listaInstrucciones, (void*) iterator);
                list_destroy_and_destroy_elements(listaInstrucciones,free);*/
                break;
            case -1:
                log_info(logger, "La consola se desconecto.");
                //continuar = accion_kernel(consola_fd, kernel_fd);
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
            consola_fd = esperar_consola(kernel_fd);
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
    int consola_fd = esperar_consola(kernel_fd);
    if (consola_fd != -1) {
        pthread_t hilo;
        t_procesar_conexion_attrs* attrs = malloc(sizeof(t_procesar_conexion_attrs));
        attrs->log = logger;
        attrs->fd = consola_fd;
        attrs->nombre_kernel = nombre_kernel;
        pthread_create(&hilo, NULL, (void*) procesar_conexion, (void*) attrs);
        pthread_detach(hilo);
        
        /*
        por default, un subproceso se ejecuta en modo unible. 
        El subproceso que se puede unir no liberará ningún recurso incluso después del final de la función
        del subproceso, hasta que otro subproceso llame a pthread_join() con su ID.

        pthread_join() es una llamada de bloqueo, bloqueará el hilo de llamada hasta que finalice el otro hilo.
        El primer parámetro de pthread_join() es el ID del hilo de destino.
        El segundo parámetro de pthread_join() es la dirección de (void *), es decir, (void **), apuntará al valor de retorno de la función de subproceso, (el puntero a (void *)).

        pthread_detach()
        es un subproceso separado que libera automaticamente los recursos asignados al salir. Ningún otro hilo necesita unirse a él. Pero por default todos los subprocesos se pueden unir, por lo que para separar un subproceso 
        debemos llamar a pthread_detach() con el id del subproceso.
        Como el subproceso separado libera automaticamente los recursos al salir, no hay forma de determinar su valor de retorno de la función de este subproceso separado.
        
        */

        return 1;
    }
    return 0;
}


void iterator(char* value) {
    log_info(logger,"%s", value);
}

t_config* iniciar_config(void) {
	t_config* nuevo_config;

	if((nuevo_config = config_create(CONFIG_FILE)) == NULL) {
		perror("No se pudo leer la configuracion: ");
		exit(-1);
	}
	return nuevo_config;

}
