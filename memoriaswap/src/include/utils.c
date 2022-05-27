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


void verificarBind(int socket_memoria, const struct addrinfo *memoriainfo) {
    if(bind(socket_memoria, memoriainfo->ai_addr, memoriainfo->ai_addrlen) == -1) {
        perror("Hubo un error en el bind: ");
        close(socket_memoria);
        exit(-1);
    }
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

void* recibirBuffer(size_t size, int socket_cpu)
{
	void * buffer;
	buffer = malloc(size);
	recv(socket_cpu, buffer, size, MSG_WAITALL);

	return buffer;
}

void recibirMensaje(int socket_cpu)
{
	int size;
	char* buffer = recibirBuffer(size, socket_cpu);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}