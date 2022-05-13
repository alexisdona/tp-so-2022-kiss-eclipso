#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include "include/utils.h"
#include <commons/process.h>

int validar_y_ejecutar_opcion_consola(int, int, int );
int recibir_opcion();
int accion_kernel(int, int);
t_pcb* crearEstructuraPcb(t_list*, int);
void iterator(char*);

#endif /* KERNEL_H_ */
