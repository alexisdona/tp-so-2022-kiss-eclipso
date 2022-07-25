#ifndef SRC_SWAP_H_
#define SRC_SWAP_H_

#include <sys/stat.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include "../../shared/headers/sharedUtils.h"

t_log* logger;
t_queue* cola_swap;
char* PATH_SWAP;
uint32_t RETARDO_SWAP;

extern int errno;

t_config* existe_archivo_swap(char* ruta);
void crear_archivo_swap(size_t, size_t);
void eliminar_archivo_swap(t_pcb* pcb);
char* obtener_ruta_archivo_swap();
void actualizar_archivo_swap(t_config* proceso_swap);
void swapear_proceso(t_pcb* pcb);
int puedo_atender_pcb();
void comenzar_swaping();
void verificar_carpeta_swap(char* ruta);

#endif
