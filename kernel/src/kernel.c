#include "kernel.h"

int main(void) {
	logger = log_create("kernel.log", "Kernel", 1, LOG_LEVEL_DEBUG);

	int kernel_fd = iniciar_kernel();
	log_info(logger, "Kernel listo para recibir una consola");
	int consola_fd = esperar_consola(kernel_fd);

	t_list* lista;
	int continuar=1;

	while (continuar) {
		int cod_op = recibir_operacion(consola_fd);
		switch (cod_op) {
			case MENSAJE:
				recibir_mensaje(consola_fd);
				break;
			case PAQUETE:
				lista = recibir_paquete(consola_fd);
				log_info(logger, "Me llegaron los siguientes valores:\n");
				list_iterate(lista, (void*) iterator);
				list_destroy_and_destroy_elements(lista,free);
				break;
			case -1:
				log_info(logger, "La consola se desconecto.");
				continuar = accion_kernel(consola_fd, kernel_fd);
				break;
			default:
				log_warning(logger,"Operacion desconocida.");
				break;
			}
	}
	return EXIT_SUCCESS;
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

	int continuar=1;

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

	if (recibir_operacion(consola_fd) == SIN_CONSOLAS) {
	  log_info(logger, "No hay mas clientes conectados.");
	  log_info(logger, "¿Desea mantener el kernel corriendo? 1- Si 0- No");
	  opcion = recibir_opcion();
	  return validar_y_ejecutar_opcion_consola(opcion, consola_fd, kernel_fd);
	}

	return validar_y_ejecutar_opcion_consola(opcion, consola_fd, kernel_fd);

}

void iterator(char* value) {
	log_info(logger,"%s", value);
}
