//
// Created by alecho on 26/5/22.
//

#ifndef TP_2022_1C_ECLIPSO_PLANIFICACION_H
#define TP_2022_1C_ECLIPSO_PLANIFICACION_H

#include <commons/collections/queue.h>
#include "../../../shared/headers/sharedUtils.h"

t_queue* READY;
t_queue* NEW;
t_queue* BLOCKED;
t_queue* SUSPENDED_READY;
t_queue* SUSPENDED_BLOCKED;
unsigned int GRADO_MULTIPROGRAMACION;
unsigned int TIEMPO_MAXIMO_BLOQUEADO;
sem_t semGradoMultiprogramacion;
pthread_mutex_t mutexColaNew;
pthread_mutex_t mutexColaReady;
pthread_mutex_t mutexColaBloqueados;
pthread_mutex_t mutexColaSuspendedBloqued;
pthread_mutex_t  mutexGradoMultiprogramacion;
int valorSemaforoContador;
int conexionMemoria;
t_log* logger;

void iniciarPlanificacionCortoPlazo(t_pcb *pcb, int, int, t_log*);
void iniciarPlanificacion(t_pcb*, t_log*, int, int);
int inicializarMutex();
void avisarProcesoTerminado(int);
void bloquearProceso(t_pcb*);
void suspenderBlockedProceso(t_pcb*);
void crear_estructuras_memoria(int, t_pcb*);
void proceso_en_ready(t_pcb*, int);

#endif //TP_2022_1C_ECLIPSO_PLANIFICACION_H
