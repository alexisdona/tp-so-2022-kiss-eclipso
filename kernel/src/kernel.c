#include "kernel.h"

int main(void) {
	logger = log_create("kernel.log", "Kernel", 1, LOG_LEVEL_DEBUG);

	int kernel_fd = iniciar_kernel();
	log_info(logger, "Kernel listo para la consola");
	int consola_fd = esperar_consola(kernel_fd);

	t_list* lista;

	while (1) {
		int cod_op = recibir_operacion(consola_fd);
		switch (cod_op) {
			case MENSAJE:
				recibir_mensaje(consola_fd);
				break;
			case PAQUETE:
				lista = recibir_paquete(consola_fd);
				log_info(logger, "Me llegaron los siguientes valores:\n");
				list_iterate(lista, (void*) iterator);
				break;
			case -1:
				log_info(logger, "El cliente se desconecto.");
				accion_kernel(consola_fd, kernel_fd);

				break;
			default:
				log_warning(logger,"Operacion desconocida.");
				break;
			}
	}
	return EXIT_SUCCESS;
}

int recibir_opcion() {
	char* opcion = NULL;
	log_info(logger, "Ingrese una opcion: ");
	scanf("%s", opcion);
	return atoi(opcion);
}

void validar_y_ejecutar_opcion_consola(int opcion, int consola_fd, int kernel_fd) {

  switch(opcion) {
	  case 1:
		  log_info(logger,"kernel continua corriendo...");
		  consola_fd = esperar_consola(kernel_fd);
		  break;
	  case 0:
		  log_info(logger,"Terminando kernel...");
		  close(kernel_fd);
		  break;
	  default: log_error(logger,"Opcion invalida. Volve a intentarlo");
			   recibir_opcion();
    }
}


void accion_kernel(int consola_fd, int kernel_fd) {
	printf("¿Desea mantener el kernel corriendo? 1- Si 0- No\n");

	int opcion = recibir_opcion();

	validar_y_ejecutar_opcion_consola(opcion, consola_fd, kernel_fd);

	if (recibir_operacion(consola_fd) == SIN_CONSOLAS) {
	  log_info(logger, "No hay mas clientes conectados, desea continuar corriendo el server?.");
	  opcion = recibir_opcion();
	  validar_y_ejecutar_opcion_consola(opcion, consola_fd, kernel_fd);
	}
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}
