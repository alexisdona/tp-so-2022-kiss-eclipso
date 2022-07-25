#include <stdio.h>
#include <stdlib.h>

#include "include/swap.h"

t_pcb* pcb_atendiendo;
t_config* pcb_swapeado;

void swapear_proceso(t_pcb* pcb){
	queue_push(cola_swap,pcb);
	if(puedo_atender_pcb()){
		comenzar_swaping();
	}
}

int puedo_atender_pcb(){
	return (pcb_atendiendo == NULL);
}

void comenzar_swaping(){
	pcb_atendiendo = queue_pop(cola_swap);
	char* ruta = obtener_ruta_archivo_swap();
	log_info(logger,string_from_format("RUTA ARCHIVO SWAP: %s\n",ruta));
	t_config* proceso_swap = existe_archivo_swap(ruta);
	actualizar_archivo_swap(proceso_swap);
	config_save(proceso_swap);
	pcb_atendiendo = NULL;
	pcb_swapeado = NULL;
}

t_config* existe_archivo_swap(char* ruta){
	t_config* proceso_swap;
	if ( (proceso_swap = config_create(ruta)) == NULL) {
	//	crear_archivo_swap(ruta);
		proceso_swap = malloc(sizeof(t_config));
		proceso_swap->path = string_duplicate(ruta);
		proceso_swap->properties = dictionary_create();
	}
	return proceso_swap;
}

char* obtener_ruta_archivo_swap(){
	char* ruta = string_new();
	string_append(&ruta, PATH_SWAP);
	string_append(&ruta, string_itoa(pcb_atendiendo->idProceso));
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
/*
    printf("\nswap --> IMPRIMO VALORES EN el archivo\n");
      for(int i=0; i< cantidad_bloques; i++) {
          uint32_t* apuntado=  str+ sizeof(uint32_t) *i;
         printf("\nvalor apuntado en posición del arhivo%d-->%d",i, *apuntado);
   }
*/
       char *ruta = string_new();
       string_append(&ruta, PATH_SWAP);
       verificar_carpeta_swap(ruta);

       string_append(&ruta, string_itoa(id_proceso));
       string_append(&ruta, ".swap");

       FILE *archivo_swap = fopen(ruta, "wb");
       if (archivo_swap != NULL) {
           fwrite(str, 1, tamanio, archivo_swap);
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


   void actualizar_archivo_swap(t_config* proceso_swap){

       dictionary_put(proceso_swap->properties,"PID",string_itoa(pcb_atendiendo->idProceso));
       dictionary_put(proceso_swap->properties,"TAM_PROCESO",string_itoa(pcb_atendiendo->tamanioProceso));
       dictionary_put(proceso_swap->properties,"PC",string_itoa(pcb_atendiendo->programCounter));
       dictionary_put(proceso_swap->properties,"TABLA_PAGINAS",string_itoa(pcb_atendiendo->tablaPaginas));
       dictionary_put(proceso_swap->properties,"ESTIMACION",string_itoa(pcb_atendiendo->estimacionRafaga));

       t_list* lista_instrucciones = pcb_atendiendo->listaInstrucciones;
       t_instruccion* instruccion;
       char* instrucciones = string_new();

       for(uint32_t i=0; i<list_size(lista_instrucciones); i++){

           instruccion = list_get(lista_instrucciones,i);
           string_append(&instrucciones,string_itoa(instruccion->codigo_operacion));
           string_append(&instrucciones," ");
           string_append(&instrucciones,string_itoa(instruccion->parametros[0]));
           string_append(&instrucciones," ");
           string_append(&instrucciones,string_itoa(instruccion->parametros[1]));
           string_append(&instrucciones,";");

       }
       dictionary_put(proceso_swap->properties,"INSTRUCCIONES",instrucciones);

   }
