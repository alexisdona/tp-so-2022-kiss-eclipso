#ifndef SRC_MEMORIA_H_
#define SRC_MEMORIA_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/list.h>

#include "include/utils.h"

#define LOG_NAME "MEMORIA_LOG"
#define LOG_FILE "memoria.log"
#define CONFIG_FILE "../src/config/memoria.config"

int recibir_opcion();
int accion_memoria(int, int);

#endif /* SRC_MEMORIA_H_ */
