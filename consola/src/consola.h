#ifndef CONSOLA_H_
#define CONSOLA_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<commons/collections/list.h>

#include "utils.h"

#define LOG_NAME "CONSOLA_LOG"
#define LOG_FILE "consola.log"
#define CONFIG_FILE "../src/consola.config"
#define CARACTER_SALIDA ""

typedef enum
{
	NO_OP,
	IO,
	READ,
	COPY,
	WRITE,
	EXIT
} instr_code;

typedef struct{
	instr_code codigo_operacion;
	uint paramteros[2];
} t_instruccion;

instr_code obtener_cop(char*);
void generar_lista_instrucciones(t_list**,char**);
void agregar_instrucciones(t_list**, char*);
char** leer_archivo_pseudocodigo(char*,t_log*);
int conectar_al_kernel();
t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void leer_consola(t_log*);
void armarPaquete(int,t_list*);
void terminar_programa(int, t_log*, t_config*);

#endif /* CONSOLA_H_ */
