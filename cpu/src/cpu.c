#include "cpu.h"
t_log* logger;
int cpuDispatch;
int cpuInterrupt;
t_config * config;
int clienteDispatch, clienteInterrupt;

int main(void) {
	logger = iniciarLogger(LOG_FILE,"CPU");
    config = iniciarConfig(CONFIG_FILE);
	char* ip= config_get_string_value(config,"IP_CPU");
    char* puertoDispatch= config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
    char* puertoInterrupt= config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");
    cpuDispatch = iniciarServidor(ip, puertoDispatch, logger);
    log_info(logger, "CPU listo para recibir un al kernel");
    clienteDispatch = esperarCliente(cpuDispatch,logger);
//	int memoria_fd = esperar_memoria(cpuDispatch);

	while(clienteDispatch!=-1) {
		op_code cod_op = recibirOperacion(clienteDispatch);
		switch (cod_op) {
			case MENSAJE:
                recibirMensaje(clienteDispatch, logger);
		        break;
		    case PCB:
                log_trace(logger, "me llego un PCB");
		        t_pcb* pcb = recibirPCB(clienteDispatch);
		        printf("pcb->idProceso: %zu\n", pcb->idProceso);
                printf("pcb->tamanioProceso: %zu\n", pcb->tamanioProceso);
                for(uint32_t i=0; i<list_size(pcb->listaInstrucciones); i++){
                    t_instruccion *instruccion = list_get(pcb->listaInstrucciones, i);
                    printf("instruccion-->codigoInstruccion->%d\toperando1-> %d\toperando2-> %d\n",
                           instruccion->codigo_operacion,
                           instruccion->parametros[0],
                           instruccion->parametros[1]);
                }
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



