//
// Created by alexis.dona on 26/5/22.
//
#include "include/planificacion.h"
void levantarGradoMultiprogramacion(){
    t_config* config = iniciar_config(CONFIG_FILE);
    gradoMultiprogramacion = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
}
void iniciarPlanificacion(){

};
