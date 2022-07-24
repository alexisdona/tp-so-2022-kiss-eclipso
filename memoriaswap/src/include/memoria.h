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
#include<commons/bitarray.h>

#define LOG_NAME "MEMORIA_LOG"
#define LOG_FILE "memoria.log"
#define CONFIG_FILE "../memoriaswap/src/config/memoria.config"
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

typedef struct {
    t_log* log;
    int fd;
    char* nombre;
} t_procesar_conexion_attrs;

typedef struct {
   uint32_t indice;
   uint32_t nro_tabla_segundo_nivel; //indice en el segundo array de tablas de segundo nivel
} t_registro_primer_nivel;

typedef struct {
    uint32_t indice;
    uint32_t frame;
    bool modificado;
    bool usado;
    bool presencia;
} t_registro_segundo_nivel;



void preparar_modulo_swap();
size_t crear_estructuras_administrativas_proceso(size_t tamanio_proceso);
void iniciar_estructuras_administrativas_kernel();
int escuchar_cliente(char *);
void crear_bitmap_frames_libres();
void* obtener_bloque_proceso_desde_swap(size_t id_proceso, uint32_t numero_pagina);

#endif /* SRC_MEMORIA_H_ */
