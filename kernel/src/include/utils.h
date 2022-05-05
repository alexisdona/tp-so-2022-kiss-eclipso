#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>
#include "protocolo.h"

#define IP "127.0.0.1"
#define PUERTO "8000"
#define SIN_CONSOLAS -1

t_log* logger;

void* recibir_buffer(int*, int);

int iniciar_kernel(void);
int esperar_consola(int);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);

#endif /* UTILS_H_ */
