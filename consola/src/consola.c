#include "consola.h"

int main(void) {

	int conexion;
	char* ip;
	int puerto;
	char* valor;

	t_log* logger;
	t_config* config;

	/* ---------------- LOGGING ---------------- */

	logger = iniciar_logger();

	log_info(logger, "Logueando...");

	/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */

	config = iniciar_config();

	ip = config_get_string_value(config,"IP_KERNEL");
	puerto = config_get_int_value(config,"PUERTO_KERNEL");
	valor = config_get_string_value(config, "CLAVE");

	log_info(logger, "IP:%s", ip);
	log_info(logger, "PUERTO:%d",puerto);
	log_info(logger, "VALOR:%s", valor);

/* ---------------- LEER DE CONSOLA ---------------- */
	printf("Log del cliente\n");
	leer_consola(logger);

	// conexion con el servidor
	conexion = crear_conexion(ip, puerto);

	enviar_mensaje("Cliente conectado.", conexion );

	enviar_mensaje(valor,conexion);

	printf("Paquete a enviar al servidor:\n");

	armarPaquete(conexion);

	terminar_programa(conexion, logger, config);
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
		leido = readline("cli> ");
		leiCaracterSalida = strcmp(leido, CARACTER_SALIDA);
		if(leiCaracterSalida!=0) log_info(logger,"cli:> %s",leido);
		free(leido);

	} while(leiCaracterSalida);

}

void armarPaquete(int conexion) {
	char* leido;
	t_paquete* paquete = crear_paquete();

	int leiCaracterSalida;

	do {
		leido = readline("cli> ");
		leiCaracterSalida = strcmp(leido, CARACTER_SALIDA);
		if(leiCaracterSalida!=0) agregar_a_paquete(paquete,leido,strlen(leido)+1);
		free(leido);
	} while(leiCaracterSalida);

	enviar_paquete(paquete,conexion);

	eliminar_paquete(paquete);
}

void terminar_programa(int conexion, t_log* logger, t_config* config) {
	log_info(logger, "Terminando programa cliente");
	log_destroy(logger);
	if(config!=NULL) config_destroy(config);
	liberar_conexion(conexion);
}
