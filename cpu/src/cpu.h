#ifndef SRC_CPU_H_
#define SRC_CPU_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include "../../shared/headers/sharedUtils.h"
#include "include/utils.h"
#include <pthread.h>

#define LOG_NAME "CPU_LOG"
#define LOG_FILE "cpu.log"
#define CONFIG_FILE "../cpu/src/config/cpu.config"

void comenzar_ciclo_instruccion();
t_instruccion* fase_fetch();
int fase_decode(t_instruccion*);
operando fase_fetch_operand(operando);
op_code fase_execute(t_instruccion* instruccion, uint32_t operador);
void operacion_NO_OP();
void operacion_IO(op_code proceso_respuesta, operando tiempo_bloqueo);
void operacion_EXIT(op_code proceso_respuesta);
void preparar_pcb_respuesta(t_paquete* paquete);
void estimar_proxima_rafaga(time_t tiempo);
int hilo_interrupcion2(char*, int);
void loggearPCB(t_pcb* pcb);
void procesar_conexion_interrupt(void*);

#endif /* SRC_CPU_H_ */
