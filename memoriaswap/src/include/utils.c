#include "utils.h"

int iniciar_memoria(void) {
	int socket_memoria;

	struct addrinfo hints, *memoriainfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(IP, PUERTO, &hints, &memoriainfo);

	// Creamos el socket de escucha de la memoria
	socket_memoria = socket(memoriainfo->ai_family,memoriainfo->ai_socktype,memoriainfo->ai_protocol);
	// Asociamos el socket a un puerto
    verificarBind(socket_memoria, memoriainfo);
    // Escuchamos las conexiones entrantes
    verificarListen(socket_memoria);

    freeaddrinfo(memoriainfo);
	log_trace(logger, "Listo para escuchar a la cpu");

	return socket_memoria;
}

void verificarListen(int socket_memoria) {
    if (listen(socket_memoria, SOMAXCONN) == -1) {
        perror("Hubo un error en el listen: ");
        close(socket_memoria);
        exit(-1);
    }
}

int esperar_cpu(int socket_memoria)
{
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


