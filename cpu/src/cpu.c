#include "cpu.h"
t_log* logger;
int cpuDispatch;
int cpuInterrupt;
t_config * config;
int conexionMemoria;
int clienteDispatch, clienteInterrupt;

int main(void) {
	logger = iniciarLogger(LOG_FILE,"CPU");
    config = iniciarConfig(CONFIG_FILE);
	char* ip= config_get_string_value(config,"IP_CPU");
	char* ipMemoria = config_get_string_value(config,"IP_MEMORIA");
	char* puertoMemoria = config_get_string_value(config,"PUERTO_MEMORIA");
    char* puertoDispatch= config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
    char* puertoInterrupt= config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");

    cpuDispatch = iniciarServidor(ip, puertoDispatch, logger);
 //   cpuInterrupt = iniciarServidor(ip, puertoInterrupt, logger);
 
 	conexionMemoria = crearConexion(ipMemoria, puertoMemoria, "Memoria");
	log_info(logger, "Te conectaste con Memoria");

    clienteDispatch = esperarCliente(cpuDispatch,logger);
//	int memoria_fd = esperar_memoria(cpuDispatch);

	while(clienteDispatch!=-1) {
		op_code cod_op = recibirOperacion(clienteDispatch);
		switch (cod_op) {
			case MENSAJE:
                recibirMensaje(clienteDispatch, logger);
		        break;
		    case PCB:
		       // recibirPcb()
		       break;
			case -1:
				log_info(logger, "El cliente se desconecto.");
				clienteDispatch=-1;
				break;
			default:
				log_warning(logger,"Operacion desconocida.");
				break;
		}
	}
	return EXIT_SUCCESS;
}



