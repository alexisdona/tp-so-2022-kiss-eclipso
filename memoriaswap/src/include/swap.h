#ifndef SRC_SWAP_H_
#define SRC_SWAP_H_

#include <sys/stat.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include "../shared/headers/sharedUtils.h"

//USER ESTE PARA LA ENTREGA
//#include "sharedUtils.h"

/* Para imprimir con colores re loco */
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

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
void inicializar_mutex_swap();
void mostrar_contenido_archivo_swap(size_t id_proceso);
#endif
