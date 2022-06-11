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
sem_t semGradoMultiprogramacion;
pthread_mutex_t mutexColaNew;
pthread_mutex_t mutexColaReady;
pthread_mutex_t  mutexGradoMultiprogramacion;
int valorSemaforoContador;

void iniciarPlanificacionCortoPlazo(t_pcb *pcb, int);
void iniciarPlanificacion(t_pcb*, t_log*, int);
int inicializarMutex();

#endif //TP_2022_1C_ECLIPSO_PLANIFICACION_H
