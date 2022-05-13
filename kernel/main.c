#include "src/main.h"

t_log* logger;
int kernel_fd;

void sighandler(int s) {
    cerrar_programa(logger);
    exit(0);
}

int main() {
    signal(SIGINT, sighandler);

    logger = log_create("kernel.log", "KERNEL", 1, LOG_LEVEL_DEBUG);

    kernel_fd = iniciar_kernel();
    log_info(logger, "Kernel listo para recibir una consola");

    while (escuchar_consolas(logger, "KERNEL", kernel_fd));

    cerrar_programa(logger);

    return 0;
}

void cerrar_programa(t_log* logger) {
	log_destroy(logger);
}
