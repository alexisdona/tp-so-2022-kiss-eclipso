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
void actualizar_archivo_swap(size_t, uint32_t, uint32_t ,uint32_t, uint32_t);
int puedo_atender_pcb();
void verificar_carpeta_swap(char* ruta);
char *obtener_path_archivo(size_t);
#endif
