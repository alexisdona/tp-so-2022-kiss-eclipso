#ifndef SRC_CPU_H_
#define SRC_CPU_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<time.h>
#include<commons/collections/list.h>
#include "../../shared/headers/sharedUtils.h"
#include "include/utils.h"

#define LOG_NAME "CPU_LOG"
#define LOG_FILE "cpu.log"
//#define CONFIG_FILE "../src/config/cpu.config"
char* CONFIG_FILE;

typedef struct {
  int cpu_interrupt;
} attrs_interrupt;

typedef struct{
	uint32_t pagina;
	uint32_t marco;
	uint32_t veces_referenciada;
} tlb_entrada;

void comenzar_ciclo_instruccion();
t_instruccion* fase_fetch();
int fase_decode(t_instruccion*);
operando fase_fetch_operand(operando);
op_code fase_execute(t_instruccion* instruccion, uint32_t operador);
op_code operacion_NO_OP();
op_code operacion_IO(operando tiempo_bloqueo);
op_code operacion_EXIT();
op_code operacion_READ(operando);
op_code operacion_WRITE(uint32_t, operando);
op_code operacion_COPY(uint32_t, uint32_t);
void preparar_pcb_respuesta(t_paquete* paquete);
void atender_interrupcion();
void loggearPCB(t_pcb* pcb);
int escuchar_interrupcion();
uint32_t tlb_obtener_marco(uint32_t entrada);
void tlb_actualizar(uint32_t numero_pagina, uint32_t marco);
void limpiar_tlb();
void handshake_memoria(int);
dir_fisica* obtener_direccion_fisica(uint32_t);
uint32_t obtener_tabla_segundo_nivel(size_t, uint32_t);
uint32_t obtener_marco_memoria(uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t leer_en_memoria(dir_fisica *);
void escribir_en_memoria(dir_fisica *, uint32_t);
static bool comparator (void*, void*);
void procesar_conexion_dispatch(void* void_args);
void procesar_conexion_interrupt(void* void_args);
void crear_hilo_dispatch();
void crear_hilo_interrupt();
void crear_hilos_cpu();
op_code chequear_interrupcion(op_code proceso_respuesta);
void levantar_configs();
void actualizar_entrada_marco_existente(uint32_t , uint32_t);
op_code matar_proceso();
void logear_direccion_fisica(dir_fisica*);

#endif /* SRC_CPU_H_ */
