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

#include "../../shared/headers/protocolo.h"

#define IP "127.0.0.1"
#define PUERTO "8000"
#define SIN_CONSOLAS -1

t_log* logger;

void* recibir_buffer(int*, int);

int iniciar_cpu(void);
int esperar_memoria(int);
op_code recibirOperacion(int);

#endif /* UTILS_H_ */
