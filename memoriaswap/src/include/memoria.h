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
#include<math.h>

#define LOG_NAME "MEMORIA_LOG"
#define LOG_FILE "memoria.log"
//#define CONFIG_FILE "../src/config/memoria.config"
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

char* CONFIG_FILE;

typedef struct {
   uint32_t indice;
   uint32_t nro_tabla_segundo_nivel; //indice en el segundo array de tablas de segundo nivel
} t_registro_primer_nivel;

typedef struct {
    uint32_t indice;
    uint32_t frame;
    uint modificado;
    uint uso;
    uint presencia;
} t_registro_segundo_nivel;

typedef struct {
	uint32_t numero_frame;
	uint32_t numero_pagina;
	uint uso;
	uint modificado;
	uint presencia;
} t_frame;

typedef struct t_frame_lista_circular {
	t_frame* info;
	struct t_frame_lista_circular* sgte;
} t_frame_lista_circular;

typedef struct {
	t_frame_lista_circular* inicio;
	t_frame_lista_circular* fin;
	int tamanio;
	t_frame_lista_circular* puntero_algoritmo;
	size_t pid;
} t_lista_circular;

pthread_mutex_t mutex_escritura;

void preparar_modulo_swap();
size_t crear_estructuras_administrativas_proceso(size_t tamanio_proceso);
void iniciar_estructuras_administrativas_kernel();
int escuchar_cliente(char *);
void crear_bitmap_frames_libres();
void* obtener_bloque_proceso_desde_swap(size_t id_proceso, uint32_t numero_pagina);
uint32_t obtener_numero_frame_libre();
uint32_t obtener_cantidad_marcos_ocupados(size_t);
uint32_t sustitucion_paginas(uint32_t, uint32_t, size_t);
uint32_t algoritmo_clock(t_lista_circular*, uint32_t, uint32_t);
uint32_t algoritmo_clock_modificado(t_lista_circular*, uint32_t, uint32_t);
uint32_t algoritmo_clock_modificado_v2(t_lista_circular*, uint32_t, uint32_t);
uint es_victima_clock(t_frame*);
uint es_victima_clock_modificado_um(t_frame*);
uint es_victima_clock_modificado_u(t_frame*);
void actualizar_registros(t_registro_segundo_nivel*, t_registro_segundo_nivel*, uint32_t);
t_lista_circular* obtener_lista_circular_del_proceso(size_t);
int es_lista_circular_del_proceso(size_t, t_lista_circular*);
t_frame_lista_circular* obtener_elemento_lista_circular(t_lista_circular*, uint32_t);
t_registro_segundo_nivel* obtener_registro_segundo_nivel(uint32_t, uint32_t);
t_lista_circular* list_create_circular();
void insertar_lista_circular_vacia(t_lista_circular*, t_frame*);
void insertar_lista_circular(t_lista_circular*, t_frame*);
void liberar_memoria_proceso(uint32_t, size_t);
void imprimir_valores_paginacion_proceso(uint32_t);
void actualizar_bit_modificado_tabla_paginas(size_t, uint32_t);
void actualizar_bit_modificado_tabla_circular(uint32_t, size_t);
void actualizar_bit_usado_tabla_paginas(size_t , uint32_t );
void inicializar_mutex_memoria();



#endif /* SRC_MEMORIA_H_ */
