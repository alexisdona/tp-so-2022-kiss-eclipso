#include "headers/sharedUtils.h"

t_config* iniciar_config(char* file) {
    t_config* nuevo_config;

    if((nuevo_config = config_create(file)) == NULL) {
        perror("No se pudo leer la configuracion: ");
        exit(-1);
    }
    return nuevo_config;

}