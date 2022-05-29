#ifndef CONSOLA_H_
#define CONSOLA_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/collections/list.h>

#include "include/utils.h"
#include "../../shared/headers/sharedUtils.h"
#include "../../shared/headers/protocolo.h"

#define LOG_NAME "CONSOLA_LOG"
#define LOG_FILE "consola.log"
#define CONFIG_FILE "../consola/src/config/consola.config"
#define CARACTER_SALIDA ""

instr_code obtener_cop(char*);
void generarListaInstrucciones(t_list **instrucciones, char **pseudocodigo);
void agregarInstrucciones(t_list **instrucciones, char *itr);
char** leer_archivo_pseudocodigo(char*,t_log*);
void enviarListaInstrucciones(uint32_t, int, t_list*);
t_list* parsearInstrucciones(t_log *logger, char *rutaArchivo);

#endif /* CONSOLA_H_ */
