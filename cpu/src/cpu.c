#include "cpu.h"

int main(int argc, char* argv[]) {
	logger = log_create("cpu.log", "CPU", 1, LOG_LEVEL_DEBUG);

	int cpu_fd = iniciar_cpu();
	log_info(logger, "CPU lista para recibir una memoria");
	int memoria_fd = esperar_memoria(cpu_fd);
	int continuar=1;

	while(continuar) {
		op_code cod_op = recibirOperacion(consola_fd);
		switch (cod_op) {
			case MENSAJE:
				recibirMensaje(memoria_fd);
				break;
			case -1:
				log_info(logger, "La memoria se desconecto.");
				continuar = accion_kernel(memoria_fd, cpu_fd);
				break;
			default:
				log_warning(logger,"Operacion desconocida.");
				break;
		}
	}
	return EXIT_SUCCESS;
}
