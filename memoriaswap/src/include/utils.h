#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <string.h>
#include <assert.h>

#include "protocolo.h"

#define IP "127.0.0.1"
#define PUERTO "8000"
#define SIN_CONSOLAS -1

t_log* logger;

void* recibir_buffer(int*, int);

int iniciar_memoria(void);
int esperar_cpu(int);
void recibirMensaje(int);
op_code recibirOperacion(int);
void verificarListen(int);

#endif /* UTILS_H_ */
