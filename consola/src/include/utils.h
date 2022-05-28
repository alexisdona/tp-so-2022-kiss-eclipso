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
#include "../../../shared/headers/sharedUtils.h"
#include<errno.h>

extern int errno;


int crear_conexion(char* ip, int puerto);


void agregarTamanioProceso(t_paquete*, int);
void agregarInstruccion(t_paquete* paqueteInstrucciones, void* instruccion);


#endif /* UTILS_H_ */
