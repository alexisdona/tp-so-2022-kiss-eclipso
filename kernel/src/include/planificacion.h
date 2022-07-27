#ifndef TP_2022_1C_ECLIPSO_PLANIFICACION_H
#define TP_2022_1C_ECLIPSO_PLANIFICACION_H

#include "../../../shared/headers/sharedUtils.h"
#include <time.h>
#include <stdbool.h>
#include <stdio.h>

t_list* READY;
t_queue* NEW;
t_queue* BLOCKED;
t_list* SUSPENDED_READY;
t_list* SUSPENDED_BLOCKED;

unsigned int GRADO_MULTIPROGRAMACION;
unsigned int MAX_GRADO_MULTIPROGRAMACION;
unsigned int TIEMPO_MAXIMO_BLOQUEADO;
sem_t semGradoMultiprogramacion;
pthread_mutex_t mutexColaNew;
pthread_mutex_t mutexColaReady;
pthread_mutex_t mutexColaBloqueados;
pthread_mutex_t mutexColaSuspendedBloqued;
pthread_mutex_t mutex_cola_suspended_ready;
pthread_mutex_t  mutexGradoMultiprogramacion;
int valorSemaforoContador;
uint32_t tiempo_max_bloqueo;
uint32_t tiempo_en_ejecucion;
t_log* logger;
int kernel_fd, conexion_cpu_dispatch, conexion_cpu_interrupt, conexion_memoria;
char* ALGORITMO_PLANIFICACION;
double ALFA;

void iniciarPlanificacionCortoPlazo();
void iniciarPlanificacion(t_pcb* pcb);
int inicializarMutex();
void avisarProcesoTerminado(int);
void bloquearProceso(t_pcb*);
void desbloquear_proceso(t_pcb*);
void suspender_proceso(t_pcb* pcb);
void estimar_proxima_rafaga(uint32_t, t_pcb*);
time_t calcular_tiempo_en_exec(time_t); 
void calcular_rafagas_restantes_proceso_desalojado(uint32_t tiempo_en_ejecucion, t_pcb* pcb_desalojada);
void ordenar_procesos_lista_READY();
void checkear_proceso_y_replanificar(t_pcb*);
void replanificar_y_enviar_nuevo_proceso(t_pcb*, t_pcb*);
void agregar_proceso_READY(t_pcb*,op_code);
void iniciar_algoritmo_planificacion(t_pcb*);
t_pcb* obtener_proceso_en_READY();
static bool sort_by_rafaga(void*, void*);
void planificacion_FIFO(t_pcb*);
void incrementar_grado_multiprogramacion();
void decrementar_grado_multiprogramacion();
void eliminar_proceso_de_READY();
unsigned int tiempo_en_suspended_blocked(t_pcb*);
void enviar_proceso_SUSPENDED_READY_a_READY();
void agregar_proceso_SUSPENDED_READY(t_pcb*);
void planificacion_SJF(t_pcb* pcb);
bool interrupcion_por_proceso_en_ready();
void crear_estructuras_memoria(t_pcb*);
void proceso_en_ready_memoria(t_pcb*);
bool hay_proceso_ejecutando();
bool altera_grado_multiprogramacion(op_code);
void seguir_algoritmo_planificacion(t_pcb*,op_code);
void continuar_planificacion();
void enviar_interrupcion(int, op_code);

#endif //TP_2022_1C_ECLIPSO_PLANIFICACION_H
