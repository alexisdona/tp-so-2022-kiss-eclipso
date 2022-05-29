#include <commons/collections/queue.h>

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

typedef struct {
    pthread_t thread;
    // ver semaforo;
} t_running_thread; // un hilo por cada planificador (corto-largo-mediano)

t_queue* READY; 
t_queue* NEW;
t_queue* BLOCKED;
t_queue* SUSPENDED_READY;
t_queue* SUSPENDED_BLOCKED;
int GRADO_MULTIPROGRAMACION;


#endif
