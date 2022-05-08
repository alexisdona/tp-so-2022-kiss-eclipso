#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include <arpa/inet.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include "protocolo.h"
#include<errno.h>

extern int errno;

typedef struct
{
	size_t size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

int crear_conexion(char* ip, int puerto);
void enviar_mensaje(char* mensaje, int socket_consola);
t_paquete* crear_paquete(void);
void agregarInstruccion(t_paquete* paqueteInstrucciones, void* instruccion);
int enviarPaquete(t_paquete* listaInstrucciones, int socket_consola);
void liberar_conexion(int socket_consola);
void eliminarPaquete(t_paquete* paquete);

#endif /* UTILS_H_ */
