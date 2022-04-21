#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include "utils.h"

void validar_y_ejecutar_opcion_consola(int opcion, int consola_fd, int kernel_fd);
int recibir_opcion();
void accion_kernel(int consola_fd, int kernel_fd);

void iterator(char* value);

#endif /* KERNEL_H_ */
