#ifndef SRC_SWAP_H_
#define SRC_SWAP_H_

#include <sys/stat.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include "../../shared/headers/sharedUtils.h"

t_log* logger;
char* PATH_SWAP;
uint32_t RETARDO_SWAP;
pthread_mutex_t mutex_swap;

extern int errno;

void crear_archivo_swap(size_t, size_t);
void eliminar_archivo_swap(size_t id_proceso);
void actualizar_archivo_swap(size_t, uint32_t, uint32_t ,uint32_t, uint32_t);
void verificar_carpeta_swap(char* ruta);
char *obtener_path_archivo(size_t);
void inicializarMutexSwap();
#endif
