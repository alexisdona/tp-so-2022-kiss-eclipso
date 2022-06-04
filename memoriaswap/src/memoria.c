#include "memoria.h"
#include "../../shared/headers/sharedUtils.h"

t_log* logger;
t_config * config;

// Variables globales
t_config* config;
t_log* logger;
int memoria_fd;

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

	while(escucharClientes(logger, "Memoria", memoria_fd));

	// Lorem ipsum

	return EXIT_SUCCESS;
}

int escucharClientes(t_log* logger, char* nombre_server, int server_socket) {
	int cliente_fd = esperarCliente(server_socket, logger);
	if (cliente_fd != -1) {
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
		return 1;
	}
	return 0;
}
