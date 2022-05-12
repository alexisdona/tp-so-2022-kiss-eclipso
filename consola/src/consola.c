#include "consola.h"

t_config* config;
t_instruccion* instruccion;

int tamanioCodigoOperacion(instr_code operacion);

int main(int argc, char* argv[]) {

    if(argc<3){
        printf("Cantidad de parametros incorrectos. Debe informar 2 parametros.\n");
        printf("1- Ruta al archivo con instrucciones a ejecutar.\n2- Tamaño del proceso\n");
        return argc;
    }

    char* rutaArchivo = argv[1];
    int tamanioProceso = atoi(argv[2]);

	uint32_t conexion;
	t_log* logger;
	logger = iniciar_logger();
	t_list* listaInstrucciones = list_create();



	char** lineasPseudocodigo = leer_archivo_pseudocodigo(rutaArchivo,logger);
	if(lineasPseudocodigo==NULL) {
		log_error(logger,"Lineas Pseudocodigo -> NULL");
		return EXIT_FAILURE;
	}

    for(uint32_t i=0; i< string_array_size(lineasPseudocodigo); i++){
    	printf("%s",lineasPseudocodigo[i]);
    }

    generarListaInstrucciones(&listaInstrucciones, lineasPseudocodigo);

	for(int i=0; i<list_size(listaInstrucciones); i++){
    	t_instruccion* instr = list_get(listaInstrucciones,i);
    	printf("instrucciones\n%d:%d:%d\n",instr->codigo_operacion,instr->parametros[0],instr->parametros[1]);
    }

	conexion = conectar_al_kernel(config);

	//enviar_mensaje("Hola soy la consola ", conexion );


    enviarListaInstrucciones(conexion, tamanioProceso, listaInstrucciones);
	string_array_destroy(lineasPseudocodigo);
	list_destroy(listaInstrucciones);
	terminar_programa(conexion, logger, config);
}


void generarListaInstrucciones(t_list** instrucciones, char** pseudocodigo){
	while(!string_array_is_empty(pseudocodigo)){
		char* instr = string_array_pop(pseudocodigo);
        agregarInstrucciones(instrucciones, instr);
	    free(instr);
	}
}

void agregarInstrucciones(t_list** instrucciones, char* itr){
	t_instruccion* instruccion = malloc(sizeof(t_instruccion));
	char* operacion = strtok_r(itr," ",&itr);
	instruccion->codigo_operacion = obtener_cop(operacion);
	switch(instruccion->codigo_operacion){
		case NO_OP:
			instruccion->parametros[0]=0;
			instruccion->parametros[1]=0;
			char* str = strtok_r(NULL,"\n",&itr);
			uint32_t repeticiones = atoi(str);
			for(uint32_t i=0; i<repeticiones; i++) list_add(*instrucciones,instruccion);
			break;
		case IO: case READ:
			instruccion->parametros[0]=atoi(strtok_r(NULL,"\n",&itr));
			instruccion->parametros[1]=0;
			list_add(*instrucciones,instruccion);
			break;
		case COPY: case WRITE:
			instruccion->parametros[0]=atoi(strtok_r(NULL," ",&itr));
			instruccion->parametros[1]=atoi(strtok_r(NULL,"\n",&itr));
			list_add(*instrucciones,instruccion);
			break;
		case EXIT:
			instruccion->parametros[0]=0;
			instruccion->parametros[1]=0;
			list_add(*instrucciones,instruccion);
			break;
	}
}

char** leer_archivo_pseudocodigo(char* ruta, t_log* logger){
	uint32_t tamMaximo = 18;
	char** lineas = string_array_new();
	char linea[tamMaximo];
	FILE* archivo = fopen(ruta,"r");
	if(archivo!=NULL){
		log_info(logger,"Archivo abierto exitosamente");
		uint32_t i=0;
	    while (fgets(linea,tamMaximo,archivo) != NULL) {
	        string_array_push(&lineas,linea);
	        lineas[i] = string_duplicate(linea);
	        i++;
	    }
	    fclose(archivo);
	    return lineas;
	}else{
		log_error(logger,"ERROR al abrir archivo PSEUDOCODIGO - Posible error en la ruta");
		return NULL;
	}
}


instr_code obtener_cop(char* operacion){
	if(string_contains(operacion,"NO_OP")) 		return NO_OP;
	else if(string_contains(operacion,"I/O")) 	return IO;
	else if(string_contains(operacion,"READ")) 	return READ;
	else if(string_contains(operacion,"COPY")) 	return COPY;
	else if(string_contains(operacion,"WRITE")) return WRITE;
	else return EXIT;
}

uint32_t conectar_al_kernel(){
	char* ip;
	uint32_t puerto;

	config = iniciar_config();
	ip = config_get_string_value(config,"IP_KERNEL");
	puerto = config_get_int_value(config,"PUERTO_KERNEL");
	uint32_t conexion = crear_conexion(ip, puerto);
	return conexion;
}

t_log* iniciar_logger(void) {

	t_log* nuevo_logger = log_create(LOG_FILE, LOG_NAME, false, LOG_LEVEL_INFO );
	return nuevo_logger;

}

t_config* iniciar_config(void) {
	t_config* nuevo_config;

	if((nuevo_config = config_create(CONFIG_FILE)) == NULL) {
		perror("No se pudo leer la configuracion: ");
		exit(-1);
	}
	return nuevo_config;

}

void leer_consola(t_log* logger) {
	char* leido;
	uint32_t leiCaracterSalida;

	do {
		leido = readline("> ");
		leiCaracterSalida = strcmp(leido, CARACTER_SALIDA);
		if(leiCaracterSalida!=0) log_info(logger,"> %s",leido);
		free(leido);

	} while(leiCaracterSalida);

}

void enviarListaInstrucciones(uint32_t conexion, int tamanioProceso, t_list* instrucciones) {
	t_paquete* paquete = crear_paquete();
	paquete->codigo_operacion = LISTA_INSTRUCCIONES;

	for(uint32_t i=0; i<list_size(instrucciones); i++){
	    t_instruccion *instruccion = list_get(instrucciones, i);
        agregarInstruccion(paquete, (void *) instruccion);
        printf("instruccion-->codigoInstruccion->%d\toperando1-> %d\toperando2-> %d\n",
               instruccion->codigo_operacion,
               instruccion->parametros[0],
               instruccion->parametros[1]);	}
    agregarTamanioProceso(paquete, tamanioProceso);
    enviarPaquete(paquete, conexion);
    eliminarPaquete(paquete);

}

int tamanioCodigoOperacion(instr_code codigoOperacion) {
    if (codigoOperacion==NO_OP || codigoOperacion== IO || codigoOperacion == READ) {
        return sizeof(instr_code) + sizeof(operando);
    }
    else if (codigoOperacion==WRITE || codigoOperacion == COPY) {
        return  sizeof(instr_code)+sizeof(operando)*2;
    }
      else {
          return sizeof(instr_code);
      }
}


void terminar_programa(uint32_t conexion, t_log* logger, t_config* config) {

	log_info(logger, "Consola: Terminando programa...");
	log_destroy(logger);
	if(config!=NULL) {
		config_destroy(config);
	}
	liberar_conexion(conexion);
}
