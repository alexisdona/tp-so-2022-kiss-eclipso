#include "utils.h"

void verificarBind(int socket_kernel, const struct addrinfo *kernelinfo);

void verificarListen(int socket_kernel);

int iniciar_kernel(void)
{
	int socket_kernel;

	struct addrinfo hints, *kernelinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(IP, PUERTO, &hints, &kernelinfo);

	// Creamos el socket de escucha del kernel
	socket_kernel = socket(kernelinfo->ai_family,kernelinfo->ai_socktype,kernelinfo->ai_protocol);
	// Asociamos el socket a un puerto
    verificarBind(socket_kernel, kernelinfo);
    // Escuchamos las conexiones entrantes
    verificarListen(socket_kernel);

    freeaddrinfo(kernelinfo);
	log_trace(logger, "Listo para escuchar a mi consola");

	return socket_kernel;
}

void verificarListen(int socket_kernel) {
    if (listen(socket_kernel, SOMAXCONN) == -1) {
        perror("Hubo un error en el listen: ");
        close(socket_kernel);
        exit(-1);
    }
}

void verificarBind(int socket_kernel, const struct addrinfo *kernelinfo) {
    if( bind(socket_kernel, kernelinfo->ai_addr, kernelinfo->ai_addrlen) == -1) {
        perror("Hubo un error en el bind: ");
        close(socket_kernel);
        exit(-1);
    } }

int esperar_consola(int socket_kernel)
{
	// Aceptamos un nuevo consola
    int socket_consola = accept(socket_kernel, NULL, NULL);

	if (socket_consola == -1) {
	    perror("Hubo un error en aceptar una conexión de la consola: ");
	    close(socket_kernel);
	    exit(-1);
	}

	log_info(logger, "Se conecto una consola!");
	return socket_consola;
}

int recibir_operacion(int socket_consola)
{
	int cod_op;

	if(recv(socket_consola, &cod_op, sizeof(int), MSG_WAITALL) > 0) {
		return cod_op;
	}
	else
	{
		close(socket_consola);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_consola)
{
	void * buffer;

	recv(socket_consola, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_consola, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_consola)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_consola);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

t_list* recibir_paquete(int socket_consola)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_consola);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}
