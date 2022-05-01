#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include "utils.h"

typedef struct {
	uint32_t proceso_id;
	uint32_t tamanio_proceso;
	t_list* listaInstrucciones;
	uint32_t program_counter;
	uint32_t tabla_paginas;
	uint32_t estimacion_rafaga;
} t_pcb;

int validar_y_ejecutar_opcion_consola(int opcion, int consola_fd, int kernel_fd);
int recibir_opcion();
int accion_kernel(int consola_fd, int kernel_fd);

void iterator(char* value);

#endif /* KERNEL_H_ */
