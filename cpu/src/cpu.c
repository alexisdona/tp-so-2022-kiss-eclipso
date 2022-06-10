#include "cpu.h"
t_log* logger;
int cpuDispatch, cpuInterrupt ;
t_config * config;
int clienteDispatch, clienteInterrupt;

void imprimirListaInstrucciones(t_pcb *pcb);

int main(void) {
	logger = iniciarLogger(LOG_FILE,"CPU");
    config = iniciarConfig(CONFIG_FILE);
	char* ip= config_get_string_value(config,"IP_CPU");
    char* puertoDispatch= config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
    char* puertoInterrupt= config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");

    cpuDispatch = iniciarServidor(ip, puertoDispatch, logger);
    log_info(logger, "CPU listo para recibir un al kernel");
    clienteDispatch = esperarCliente(cpuDispatch,logger);
    char* ipMemoria= config_get_string_value(config,"IP_MEMORIA");
    int puertoMemoria= config_get_int_value(config,"PUERTO_MEMORIA");
 //   int conexionMemoria = crearConexion(ipMemoria, puertoMemoria, "Kernel");
  //  enviarMensaje("Hola memoria soy el CPU", conexionMemoria);

    while(clienteDispatch!=-1) {
		op_code cod_op = recibirOperacion(clienteDispatch);
		switch (cod_op) {
			case MENSAJE:
                recibirMensaje(clienteDispatch, logger);
		        break;
		    case PCB:
                enviarMensaje("** CPU **Recibi el PCB, termino de ejecutar y te aviso.", clienteDispatch);
                log_info(logger, "me llego un PCB");
		        t_pcb* pcb = recibirPCB(clienteDispatch);
                printf("clienteDispatch: %d\n", clienteDispatch);
                printf("pcb->consola_fd: %zu\n", pcb->consola_fd);
                printf("pcb->kernel_fd: %zu\n", pcb->kernel_fd);
                imprimirListaInstrucciones(pcb); // esto es para tener referencia, después eliminar esta funcion

                sleep(5); //simulo tiempo en executing y le aviso al kernel para que lo termine
                enviarPCB(pcb->kernel_fd, pcb, TERMINAR_PROCESO);
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

void imprimirListaInstrucciones(t_pcb *pcb) {
    for(uint32_t i=0; i < list_size(pcb->listaInstrucciones); i++){
        t_instruccion *instruccion = list_get(pcb->listaInstrucciones, i);
        printf("instruccion-->codigoInstruccion->%d\toperando1-> %d\toperando2-> %d\n",
               instruccion->codigo_operacion,
               instruccion->parametros[0],
               instruccion->parametros[1]);
    }
}



