#include "memoria.h"
#include "../../shared/headers/sharedUtils.h"

t_log* logger;
t_config * config;

// Variables globales
t_config* config;
t_log* logger;
int memoria_fd;
int cliente_fd;

int main(void) {
	//sem_t semMemoria;

	config = iniciarConfig(CONFIG_FILE);
	logger = iniciarLogger("memoria.log", "Memoria");
	char* ipMemoria= config_get_string_value(config,"IP_MEMORIA");
    char* puertoMemoria= config_get_string_value(config,"PUERTO_ESCUCHA");

	//sem_init(&semMemoria, 0, 1);

	// Inicio el servidor
	memoria_fd = iniciarServidor(ipMemoria, puertoMemoria, logger);
	log_info(logger, "Memoria lista para recibir a Kernel o CPU");
   // cliente_fd = esperarCliente(memoria_fd, logger);

    while (1) {
        escucharClientes("CPU");
        escucharClientes("KERNEL");
    }

	return EXIT_SUCCESS;
}

void procesar_conexion(void* void_args) {

    t_procesar_conexion_attrs* attrs = (t_procesar_conexion_attrs*) void_args;
	t_log* logger = attrs->log;
    int cliente_fd = attrs->fd;
    free(attrs);

      while(cliente_fd != -1) {
	        op_code cod_op = recibirOperacion(cliente_fd);
            switch (cod_op) {
                case MENSAJE:
                    recibirMensaje(cliente_fd, logger);
                    break;
                case ESCRIBIR_MEMORIA:
                    enviarMensaje("Voy a escribir en memoria...", cliente_fd);
                    // sem_wait(&semMemoria);
                    // Escribir en memoria...
                    // sem_signal(&semMemoria);
                    enviarMensaje("Ya escribí en memoria!", cliente_fd);
                    break;
                case LEER_MEMORIA:
                    enviarMensaje("Voy a leer la memoria...", cliente_fd);
                    // sem_wait(&semMemoria);
                    // Leer memoria...
                    // sem_signal(&semMemoria);
                    enviarMensaje("Ya leí la memoria!", cliente_fd);
                    break;
                case -1:
                    log_info(logger, "El cliente se desconectó");
                    cliente_fd = -1;
                    break;
                default:
                    log_warning(logger, "Operacion desconocida.");
                    break;
            }

	}
}

int escucharClientes(char *nombre_cliente) {
    int cliente = esperarCliente(memoria_fd, logger);
    if (cliente != -1) {
        pthread_t hilo;
        t_procesar_conexion_attrs* attrs = malloc(sizeof(t_procesar_conexion_attrs));
        attrs->log = logger;
        attrs->fd = cliente;
        attrs->nombre_kernel = nombre_cliente;
        pthread_create(&hilo, NULL, (void*) procesar_conexion, (void*) attrs);
        log_info(logger, "Se creo el hilo para recibir a %s", nombre_cliente);
        pthread_detach(hilo);
        return 1;
    }
    return 0;
}


