#ifndef SRC_MEMORIA_H_
#define SRC_MEMORIA_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<semaphore.h>
#include<pthread.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/list.h>

#include "include/utils.h"

#define LOG_NAME "MEMORIA_LOG"
#define LOG_FILE "memoria.log"
#define CONFIG_FILE "../memoriaswap/src/config/memoria.config"

typedef struct {
    t_log* log;
    int fd;
    char* nombre;
} t_procesar_conexion_attrs;

int escucharClientes(t_log*, char*, int);

#endif /* SRC_MEMORIA_H_ */
