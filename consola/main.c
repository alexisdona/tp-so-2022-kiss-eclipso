#include "src/consola.h"

t_config* config;

int main(int argc, char* argv[]) {

	t_log* logger = iniciar_logger();
	uint32_t conexion = conectar_al_kernel(config);

  if(argc < 3){
		printf("Cantidad de parametros incorrectos. Debe informar 2 parametros.\n");
		printf("1- Ruta al archivo con instrucciones a ejecutar.\n2- Tamaño del proceso\n");
		return argc;
	}

	char* rutaArchivo = argv[1];
	int tamanioProceso = atoi(argv[2]);

	recibirInstrucciones(conexion, logger, rutaArchivo, tamanioProceso);

	return EXIT_SUCCESS;
}
