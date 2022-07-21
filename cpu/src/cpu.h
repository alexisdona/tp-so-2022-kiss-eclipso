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

#define LOG_NAME "CPU_LOG"
#define LOG_FILE "cpu.log"
#define CONFIG_FILE "../src/config/cpu.config"


typedef struct {
  int cpu_interrupt;
} attrs_interrupt;

typedef struct{
	uint32_t pagina;
	uint32_t marco;
} tlb_entrada;

void comenzar_ciclo_instruccion();
t_instruccion* fase_fetch();
int fase_decode(t_instruccion*);
operando fase_fetch_operand(operando);
op_code fase_execute(t_instruccion* instruccion, uint32_t operador);
void operacion_NO_OP();
void operacion_IO(op_code proceso_respuesta, operando tiempo_bloqueo);
void operacion_EXIT(op_code proceso_respuesta);
void operacion_READ(operando);
void preparar_pcb_respuesta(t_paquete* paquete);
void atender_interrupcion(void* void_args);
void loggearPCB(t_pcb* pcb);
int escuchar_interrupcion();
uint32_t pedir_a_memoria_num_tabla_segundo_nivel(uint32_t dato);
uint32_t pedir_a_memoria_marco(uint32_t dato,uint32_t dato2);
dir_fisica* traducir_direccion_logica(uint32_t dir_logica);
uint32_t tlb_obtener_marco(uint32_t entrada);
void tlb_actualizar(uint32_t numero_pagina, uint32_t marco);
uint32_t tlb_existe(uint32_t numero_pagina);
void limpiar_tlb();
void imprimirListaInstrucciones(t_pcb *pcb);

#endif /* SRC_CPU_H_ */
