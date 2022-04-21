#include "utils.h"

int iniciar_kernel(void)
{
	int socket_kernel, socket_conexion;

	struct addrinfo hints, *kernelinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(IP, PUERTO, &hints, &kernelinfo);

	// Creamos el socket de escucha del kernel
	socket_kernel = socket(kernelinfo->ai_family,kernelinfo->ai_socktype,kernelinfo->ai_protocol);
	// Asociamos el socket a un puerto
	bind(socket_kernel,kernelinfo->ai_addr,kernelinfo->ai_addrlen);
	// Escuchamos las conexiones entrantes
	listen(socket_kernel,SOMAXCONN);

	freeaddrinfo(kernelinfo);
	log_trace(logger, "Listo para escuchar a mi consola");

	return socket_kernel;
}

int esperar_consola(int socket_kernel)
{
	// Aceptamos un nuevo consola
	int socket_consola = accept(socket_kernel, NULL, NULL);
	log_info(logger, "Se conecto una consola!");

	return socket_consola;
}

int recibir_operacion(int socket_consola)
{
	int cod_op;
	if(recv(socket_consola, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
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
