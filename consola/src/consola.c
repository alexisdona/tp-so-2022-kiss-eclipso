#include "consola.h"

t_config* config;
t_instruccion* instruccion;

int main(int argc, char* argv[]) {

    if(argc<3){
        printf("Cantidad de parametros incorrectos.\n");
        printf("1-ruta archivo pseudocodigo 2-tamaño\n");
        return argc;
    }

	int conexion;
	t_log* logger;
	logger = iniciar_logger();
	t_list* listaInstrucciones = list_create();

	char** lineasPseudocodigo = leer_archivo_pseudocodigo(argv[1],logger);
	if(lineasPseudocodigo==NULL) {
		log_error(logger,"Lineas Pseudocodigo -> NULL");
		return EXIT_FAILURE;
	}

    for(int i=0; i<string_array_size(lineasPseudocodigo); i++){
    	printf("%s",lineasPseudocodigo[i]);
    }

	generar_lista_instrucciones(&listaInstrucciones,lineasPseudocodigo);

	for(int i=0; i<list_size(listaInstrucciones); i++){
    	t_instruccion* instr = list_get(listaInstrucciones,i);
    	printf("%d:%d:%d\n",instr->codigo_operacion,instr->paramteros[0],instr->paramteros[1]);
    }

	conexion = conectar_al_kernel(config);

	enviar_mensaje("Consola conectada.", conexion );

	printf("Paquete a enviar al kernel:\n");
	//armarPaquete(conexion,listaInstrucciones);
	string_array_destroy(lineasPseudocodigo);
	list_destroy(listaInstrucciones);
	terminar_programa(conexion, logger, config);
}


void generar_lista_instrucciones(t_list** instrucciones,char** pseudocodigo){
	while(!string_array_is_empty(pseudocodigo)){
		char* instr = string_array_pop(pseudocodigo);
	    agregar_instrucciones(instrucciones,instr);
	    free(instr);
	}
}

void agregar_instrucciones(t_list** instrucciones, char* itr){
	t_instruccion* instruccion = malloc(sizeof(t_instruccion));
	char* operacion = strtok_r(itr," ",&itr);
	instruccion->codigo_operacion = obtener_cop(operacion);
	switch(instruccion->codigo_operacion){
		case NO_OP:
			instruccion->paramteros[0]=0;
			instruccion->paramteros[1]=0;
			char* str = strtok_r(NULL,"\n",&itr);
			int repeticiones = atoi(str);
			for(int i=0; i<repeticiones; i++) list_add(*instrucciones,instruccion);
			break;
		case IO: case READ:
			instruccion->paramteros[0]=atoi(strtok_r(NULL,"\n",&itr));
			instruccion->paramteros[1]=0;
			list_add(*instrucciones,instruccion);
			break;
		case COPY: case WRITE:
			instruccion->paramteros[0]=atoi(strtok_r(NULL," ",&itr));
			instruccion->paramteros[1]=atoi(strtok_r(NULL,"\n",&itr));
			list_add(*instrucciones,instruccion);
			break;
		case EXIT:
			instruccion->paramteros[0]=0;
			instruccion->paramteros[1]=0;
			list_add(*instrucciones,instruccion);
			break;
	}
}

char** leer_archivo_pseudocodigo(char* ruta, t_log* logger){
	int tamMaximo = 18;
	char** lineas = string_array_new();
	char linea[tamMaximo];

	FILE* archivo = fopen(ruta,"r");
	if(archivo!=NULL){
		log_info(logger,"Archivo abierto exitosamente");
		int i=0;
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

int conectar_al_kernel(){
	char* ip;
	int puerto;

	config = iniciar_config();
	ip = config_get_string_value(config,"IP_KERNEL");
	puerto = config_get_int_value(config,"PUERTO_KERNEL");
	int conexion = crear_conexion(ip, puerto);
	return conexion;
}

t_log* iniciar_logger(void) {

	t_log* nuevo_logger = log_create(LOG_FILE, LOG_NAME, false, LOG_LEVEL_INFO );
	return nuevo_logger;

}

t_config* iniciar_config(void) {
	t_config* nuevo_config;

	if((nuevo_config = config_create(CONFIG_FILE)) == NULL) {
		printf("No se pudo leer la configuracion.\n");
		exit(2);
	}
	return nuevo_config;

}

void leer_consola(t_log* logger) {
	char* leido;
	int leiCaracterSalida;

	do {
		leido = readline("> ");
		leiCaracterSalida = strcmp(leido, CARACTER_SALIDA);
		if(leiCaracterSalida!=0) log_info(logger,"> %s",leido);
		free(leido);

	} while(leiCaracterSalida);

}

void armarPaquete(int conexion, t_list* instrucciones) {
	t_paquete* paquete = crear_paquete();

	for(int i=0; i<list_size(instrucciones); i++){
		agregar_a_paquete(paquete,list_get(instrucciones,i),3*sizeof(int));
	}

	enviar_paquete(paquete,conexion);
	eliminar_paquete(paquete);

}

void terminar_programa(int conexion, t_log* logger, t_config* config) {

	log_info(logger, "Consola: Terminando programa...");
	log_destroy(logger);
	if(config!=NULL) {
		config_destroy(config);
	}
	liberar_conexion(conexion);
}
