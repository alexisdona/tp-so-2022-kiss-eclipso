#include "kernel.h"

void validar_y_ejecutar_opcion_consola(int opcion, int cliente_fd, int server_fd);
int recibir_opcion();
void accion_servidor(int cliente_fd, int server_fd);

int main(void) {
	logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);

	int server_fd = iniciar_servidor();
	log_info(logger, "Servidor listo para recibir al cliente");
	int cliente_fd = esperar_cliente(server_fd);

	t_list* lista;

	while (1) {
		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op) {
			case MENSAJE:
				recibir_mensaje(cliente_fd);
				break;
			case PAQUETE:
				lista = recibir_paquete(cliente_fd);
				log_info(logger, "Me llegaron los siguientes valores:\n");
				list_iterate(lista, (void*) iterator);
				break;
			case -1:
				log_info(logger, "El cliente se desconecto.");

				accion_servidor(cliente_fd, server_fd);

				break;
			default:
				log_warning(logger,"Operacion desconocida.");
				break;
			}
	}
	return EXIT_SUCCESS;
}

int recibir_opcion() {
	char* opcion;
	log_info(logger, "Ingrese una opcion: ");
	scanf("%s", opcion);
	return atoi(opcion);
}

void validar_y_ejecutar_opcion_consola(int opcion, int cliente_fd, int server_fd) {

  switch(opcion) {
	  case 1:
		  log_info(logger,"Servidor continua corriendo...");
		  cliente_fd = esperar_cliente(server_fd);
		  break;
	  case 0:
		  log_info(logger,"Terminando servidor...");
		  close(server_fd);
		  break;
	  default: log_error(logger,"Opcion invalida. Volve a intentarlo");
			   recibir_opcion();
    }
}


void accion_servidor(int cliente_fd, int server_fd) {
	printf("¿Desea mantener el servidor corriendo? 1- Si 0- No\n");

	int opcion = recibir_opcion();

	validar_y_ejecutar_opcion_consola(opcion, cliente_fd, server_fd);

	if (recibir_operacion(cliente_fd) == SIN_CLIENTES) {
	  log_info(logger, "No hay mas clientes conectados, desea continuar corriendo el server?.");
	  opcion = recibir_opcion();
	  validar_y_ejecutar_opcion_consola(opcion, cliente_fd, server_fd);
	}
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}
