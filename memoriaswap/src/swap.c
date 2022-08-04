#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "include/swap.h"

void inicializarMutexSwap() {
    if(pthread_mutex_init(&mutex_swap, NULL) != 0) {
        perror("Mutex swap falló: ");
    }
}

char *obtener_path_archivo(size_t id_proceso) {

    char* ruta = string_new();
    string_append(&ruta, PATH_SWAP);
    verificar_carpeta_swap((ruta));

    string_append(&ruta, string_itoa(id_proceso));
    string_append(&ruta, ".swap");

    return ruta;
}


void crear_archivo_swap(size_t id_proceso, size_t tamanio) {

    void *str = malloc(tamanio);
    uint32_t valor= (uint32_t) id_proceso;
    uint32_t cantidad_bloques = tamanio/sizeof(uint32_t);
    // lleno las páginas del proceso con enteros partiendo desde id_proceso
    for(int i=0; i< cantidad_bloques ; i++) {
        memcpy(str + sizeof(uint32_t) *i , &valor, sizeof(uint32_t));
        valor += 1;
    }

    printf("\nswap --> IMPRIMO VALORES EN el archivo\n");
      for(int i=0; i< cantidad_bloques; i++) {
          uint32_t* apuntado=  str+ sizeof(uint32_t) *i;
         printf("\nvalor apuntado en posición del arhivo%d-->%d\n",i, *apuntado);
   }

       char *ruta = obtener_path_archivo(id_proceso);

       FILE *archivo_swap = fopen(ruta, "wb");
       if (archivo_swap != NULL) {
           pthread_mutex_lock(&mutex_swap);
           fwrite(str, 1, tamanio, archivo_swap);
           pthread_mutex_unlock(&mutex_swap);
       }
       else {
           perror("Error abriendo el archivo: ");
       }
       fclose(archivo_swap);
   }
   void verificar_carpeta_swap(char* ruta){
       log_info(logger,PATH_SWAP);
       mode_t modo_carpeta = 0777;
       int estado = mkdir(PATH_SWAP, modo_carpeta);
       if (estado == 0){
           log_info(logger,"SE CREO LA CARPETA SWAP EXITOSAMENTE");
       }/*
       else{
           log_info(logger,"La carpeta ya existe");
           //perror("La carpeta ya existe, se guardará el archivo dentro: ");
       }*/
   }

   void actualizar_archivo_swap(size_t id_proceso, uint32_t numero_pagina, uint32_t desplazamiento, uint32_t tamanio_pagina, uint32_t valor){

      int ubicacion_valor_reemplazo = numero_pagina*tamanio_pagina+desplazamiento;
       char *ruta = obtener_path_archivo(id_proceso);

       int archivo_swap = open(ruta, O_RDWR);
       struct stat sb;
       if (fstat(archivo_swap,&sb) == -1) {
           perror("No se pudo obtener el size del archivo swap: ");
       }
       void* contenido_swap = mmap(NULL, sb.st_size, PROT_WRITE , MAP_SHARED, archivo_swap, 0);
       uint32_t * apuntado = (uint32_t *) (contenido_swap+ubicacion_valor_reemplazo);
       printf("\nACTUALIZAR SWAP - ANTES DE REEPLAZO OFFSET: %d VALOR: %d\n", ubicacion_valor_reemplazo, *apuntado );
       void* valor_a_copiar = &valor;

       memcpy((contenido_swap+ubicacion_valor_reemplazo), valor_a_copiar, sizeof(uint32_t));
       apuntado = (uint32_t *) (contenido_swap+ubicacion_valor_reemplazo);
       printf("\nACTUALIZAR SWAP - DESPUES DE REEPLAZO OFFSET: %d VALOR: %d\n", ubicacion_valor_reemplazo, *apuntado );
       munmap(contenido_swap, sb.st_size);
       msync(contenido_swap, sb.st_size, MS_SYNC);
       close(archivo_swap);
    /*  Solo para corroborar que el archivo se actualizo
     *
     * FILE *archivo_swap2 = fopen(ruta, "rb");
       if (archivo_swap != NULL) {
           void* lectura = malloc(sb.st_size);
           fread(lectura, sizeof(uint32_t), sb.st_size, archivo_swap2);
           for(int i=0; i< sb.st_size/sizeof (uint32_t); i++) {
               uint32_t* apuntado=  lectura+ sizeof(uint32_t) *i;
               printf("\nvalor apuntado en posición del arhivo actualizado%d-->%d\n",i, *apuntado);
           }
       }
*/
   }

void eliminar_archivo_swap(size_t id_proceso){

    char* ruta = obtener_path_archivo(id_proceso);
    pthread_mutex_lock(&mutex_swap);
    if(remove(ruta)!=0){
        perror(string_from_format("Hubo un error borrando el archivo swap del proceso %d", id_proceso));
    }
    pthread_mutex_unlock(&mutex_swap);
}
