#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <signal.h>
#include <inttypes.h>
#include <commons/log.h>
#include "include/utils.h"
#include <commons/process.h>
#include <commons/collections/queue.h>
#include "include/estructuras.h"

#include "../../shared/headers/sharedUtils.h"

#define CONFIG_FILE "../kernel/src/config/kernel.config"



int validar_y_ejecutar_opcion_consola(int opcion, int consola_fd, int kernel_fd);
int recibir_opcion();
int accion_kernel(int, int);
t_pcb* crearEstructuraPcb(t_list*, int, int);
void iterator(char*);
int escucharClientes( char *nombre_kernel);
void iterator(char*);
int validar_y_ejecutar_opcion_consola(int, int, int );
void enviarACpu(int, t_pcb*);
#endif /* KERNEL_H_ */
