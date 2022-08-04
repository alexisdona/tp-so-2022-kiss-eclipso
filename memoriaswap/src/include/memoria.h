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


/* Para imprimir con colores re loco */
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"


#define LOG_NAME "MEMORIA_LOG"
#define LOG_FILE "memoria.log"
#define CONFIG_FILE "../memoriaswap/src/config/memoria.config"
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

typedef struct {
   uint32_t indice;
   uint32_t nro_tabla_segundo_nivel; //indice en el segundo array de tablas de segundo nivel
} t_registro_primer_nivel;

typedef struct {
    uint32_t indice;
    uint32_t frame;
    uint modificado;
    uint usado;
    uint presencia;
} t_registro_segundo_nivel;

pthread_mutex_t &mutexEscritura;

void preparar_modulo_swap();
size_t crear_estructuras_administrativas_proceso(size_t tamanio_proceso);
void iniciar_estructuras_administrativas_kernel();
int escuchar_cliente(char *);
void crear_bitmap_frames_libres();
void* obtener_bloque_proceso_desde_swap(size_t id_proceso, uint32_t numero_pagina);
uint32_t obtener_numero_frame_libre();
uint32_t obtener_cantidad_marcos_ocupados(size_t);
void cargar_pagina(uint32_t );
void algoritmo_clock_modificado(uint32_t, uint32_t, uint32_t);
void algoritmo_clock(uint32_t, uint32_t, uint32_t, uint32_t);
void liberar_memoria_proceso(uint32_t, size_t);
void imprimir_valores_paginacion_proceso(uint32_t);
void actualizar_bit_modificado_tabla_paginas(size_t, uint32_t);


#endif /* SRC_MEMORIA_H_ */
