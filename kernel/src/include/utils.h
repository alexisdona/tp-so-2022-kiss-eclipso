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

#define SIN_CONSOLAS -1

t_log* logger;
extern int errno;

t_list* recibirListaInstrucciones(int);
t_list* deserializarListaInstrucciones(void *pVoid, size_t tamanioListaInstrucciones, t_list *instrucciones);
int recibirTamanioProceso(int);
void cerrar_programa(t_log* logger);


#endif /* UTILS_H_ */
