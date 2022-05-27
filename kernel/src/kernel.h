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
#include <commons/config.h>
#include <commons/process.h>
#include <commons/collections/queue.h>
#include "include/estructuras.h"

#define CONFIG_FILE "../src/config/kernel.config"

int validar_y_ejecutar_opcion_consola(int, int, int );

typedef struct {
    t_log* log;
    int fd;
    char* nombre_kernel;
} t_procesar_conexion_attrs;

typedef enum {
    NEW                 = 'N',
    READY               = 'R',
    EXEC                = 'X',
    BLOCKED             = 'B',
    SUSPENDED_READY     = 'S',
    SUSPENDED_BLOCKED   = 'L',
    EXIT                = 'E',
} t_status;

bool PLANIFICADOR_HABILITADO;
bool PLANIFICACION_BLOQUEADA;

typedef struct {
    t_pcb* pcb;
    pthread_t thread;
    // ver semaforo;
    bool blocked;
}

t_config* iniciar_config(void);
int validar_y_ejecutar_opcion_consola(int opcion, int consola_fd, int kernel_fd);
int recibir_opcion();
int accion_kernel(int, int);
t_pcb* crearEstructuraPcb(t_list*, int);
void iterator(char*);
int escuchar_consolas(t_log*, char*, int);
void iterator(char*);

#endif /* KERNEL_H_ */
