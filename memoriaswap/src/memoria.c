#include "memoria.h"

int main(void) {
	logger = log_create("memoria.log", "Memoria", 1, LOG_LEVEL_DEBUG);

	int memoria_fd = iniciar_memoria();
	log_info(logger, "Memoria lista para recibir una cpu");
	int cpu_fd = esperar_cpu(memoria_fd);
	int continuar=1;

	while(continuar) {
		op_code cod_op = recibirOperacion(cpu_fd);
		switch (cod_op) {
			case MENSAJE:
				recibirMensaje(cpu_fd);
				break;
			case LISTA_INSTRUCCIONES:
				// Recibir instrucciones y devolverlas
				break;
			case -1:
				log_info(logger, "La cpu se desconecto.");
				continuar = accion_memoria(cpu_fd, memoria_fd);
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

int validar_y_ejecutar_opcion_cpu(int opcion, int cpu_fd, int memoria_fd) {

	int continuar=1;

	switch(opcion) {
		case 1:
			log_info(logger,"Memoria continua corriendo, esperando nueva cpu.");
			cpu_fd = esperar_cpu(memoria_fd);
			break;
		case 0:
			log_info(logger,"Terminando memoria...");
			close(memoria_fd);
			continuar = 0;
			break;
		default:
			log_error(logger,"Opcion invalida. Volve a intentarlo");
			recibir_opcion();
	}

	return continuar;

}

int accion_memoria(int cpu_fd, int memoria_fd) {

	log_info(logger, "¿Desea mantener la memoria corriendo? 1- Si 0- No");
	int opcion = recibir_opcion();

	if (recibirOperacion(cpu_fd) == 0) {
	  log_info(logger, "No hay mas clientes conectados.");
	  log_info(logger, "¿Desea mantener la memoria corriendo? 1- Si 0- No");
	  opcion = recibir_opcion();
	  return validar_y_ejecutar_opcion_cpu(opcion, cpu_fd, memoria_fd);
	}

	return validar_y_ejecutar_opcion_cpu(opcion, cpu_fd, memoria_fd);

}
