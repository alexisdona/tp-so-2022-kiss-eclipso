#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include <commons/string.h>
#include<string.h>
#include<assert.h>
#include<errno.h>
#include "../../../shared/headers/sharedUtils.h"
#include "../../../shared/headers/protocolo.h"

#define IP "127.0.0.1"
#define PUERTO "8000"
#define SIN_CONSOLAS -1

t_log* logger;
extern int errno;


void* recibirBuffer(size_t, int);
int iniciar_kernel(void);
int esperar_consola(int);
t_list* recibirListaInstrucciones(int);
int recibirTamanioProceso(int);
void recibirMensaje(int);
op_code recibirOperacion(int);
void verificarBind(int, const struct addrinfo *);
void verificarListen(int);


#endif /* UTILS_H_ */
