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

#define LOG_NAME "MEMORIA_LOG"
#define LOG_FILE "memoria.log"
#define CONFIG_FILE "../src/config/memoria.config"

typedef struct {
    t_log* log;
    int fd;
    char* nombre;
} t_procesar_conexion_attrs;


void preparar_modulo_swap();

#endif /* SRC_MEMORIA_H_ */
