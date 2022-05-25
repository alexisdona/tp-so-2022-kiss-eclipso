#include "cpu.h"

int main(void) {
	logger = log_create("cpu.log", "CPU", 1, LOG_LEVEL_DEBUG);

	int cpu_fd = iniciar_cpu();
	log_info(logger, "CPU lista para recibir una memoria");
	int memoria_fd = esperar_memoria(cpu_fd);
	int continuar=1;

	while(continuar) {
		op_code cod_op = recibirOperacion(memoria_fd);
		switch (cod_op) {
			case MENSAJE:
				recibirMensaje(memoria_fd);
				break;
			case -1:
				log_info(logger, "La memoria se desconecto.");
				continuar = accion_cpu(memoria_fd, cpu_fd);
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

int validar_y_ejecutar_opcion_memoria(int opcion, int memoria_fd, int cpu_fd) {

	int continuar=1;

	switch(opcion) {
		case 1:
			log_info(logger,"cpu continua corriendo, esperando nueva memoria.");
			memoria_fd = esperar_memoria(cpu_fd);
			break;
		case 0:
			log_info(logger,"Terminando cpu...");
			close(cpu_fd);
			continuar = 0;
			break;
		default:
			log_error(logger,"Opcion invalida. Volve a intentarlo");
			recibir_opcion();
	}

	return continuar;

}

int accion_cpu(int memoria_fd, int cpu_fd) {

	log_info(logger, "¿Desea mantener el kernel corriendo? 1- Si 0- No");
	int opcion = recibir_opcion();

	if (recibirOperacion(memoria_fd) == SIN_CONSOLAS) {
	  log_info(logger, "No hay mas clientes conectados.");
	  log_info(logger, "¿Desea mantener el kernel corriendo? 1- Si 0- No");
	  opcion = recibir_opcion();
	  return validar_y_ejecutar_opcion_memoria(opcion, memoria_fd, cpu_fd);
	}

	return validar_y_ejecutar_opcion_memoria(opcion, memoria_fd, cpu_fd);

}
