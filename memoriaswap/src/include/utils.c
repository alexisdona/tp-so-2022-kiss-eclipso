#include "utils.h"

int esperar_cpu(int socket_memoria) {
	// Aceptamos una cpu
    int socket_cpu = accept(socket_memoria, NULL, NULL);

	if (socket_cpu == -1) {
	    perror("Hubo un error en aceptar una conexión de la cpu: ");
	    close(socket_memoria);
	    exit(-1);
	}

	log_info(logger, "Se conecto una cpu!");
	return socket_cpu;
}

/*int escuchar_cpu(t_log* logger, char* nombre_memoria, char* nombre_cpu, int socket_memoria) {
    int cpu_fd = esperar_cliente(socket_memoria, nombre_cpu, logger);

    if (cpu_fd != -1) {
        pthread_t hilo;
        t_procesar_conexion_attrs* attrs = malloc(sizeof(t_procesar_conexion_attrs));
        attrs->log = logger;
        attrs->fd = cpu_fd;
        attrs->nombre_servidor = nombre_memoria;
        pthread_create(&hilo, NULL, (void*) procesar_conexion, (void*) args);
        pthread_detach(hilo);
        return 1;
    }
    return 0;
}*/

op_code recibirOperacion(int socket_cpu) {
    op_code cod_op;

    if(recv(socket_cpu, &cod_op, sizeof(op_code), MSG_WAITALL) > 0) {
        printf("recibirOperacion --> cod_op: %d\n", cod_op);
        return cod_op;
    }
    else {
        close(socket_cpu);
        return -1;
    }
}
