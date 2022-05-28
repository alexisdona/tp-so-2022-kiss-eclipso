#ifndef CONSOLA_H_
#define CONSOLA_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include<readline/readline.h>
#include<commons/collections/list.h>


#include "include/utils.h"
#include "../../shared/headers/sharedUtils.h"

#define LOG_NAME "CONSOLA_LOG"
#define LOG_FILE "consola.log"
#define CONFIG_FILE "../consola/src/config/consola.config"
#define CARACTER_SALIDA ""



instr_code obtener_cop(char*);
void generarListaInstrucciones(t_list **instrucciones, char **pseudocodigo);
void agregarInstrucciones(t_list **instrucciones, char *itr);
char** leer_archivo_pseudocodigo(char*,t_log*);
void leer_consola(t_log*);
void enviarListaInstrucciones(uint32_t, int, t_list*);
int recibirInstrucciones(uint32_t, t_log*, char*, int);
int tamanioCodigoOperacion(instr_code operacion);

#endif /* CONSOLA_H_ */
