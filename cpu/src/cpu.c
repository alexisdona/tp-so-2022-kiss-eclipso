#include "cpu.h"

t_log* logger;
int cpuDispatch;
int cpuInterrupt;
t_config * config;
int conexionMemoria;
int clienteDispatch, clienteInterrupt;
t_pcb * pcb;
int retardo_noop;
int cpu_dispatch;
int cliente_dispatch;

void imprimirListaInstrucciones(t_pcb *pcb);

int main(void) {

	logger = iniciarLogger(LOG_FILE,"CPU");
    config = iniciarConfig(CONFIG_FILE);

	char* ip = config_get_string_value(config,"IP_CPU");
	//char* ip_memoria = config_get_string_value(config,"IP_MEMORIA");

	//int puerto_memoria = config_get_int_value(config,"PUERTO_MEMORIA");
    char* puerto_dispatch = config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
    char* puerto_interrupt = config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");
    retardo_noop = config_get_int_value(config,"RETARDO_NOOP");
    int tiempo_bloqueo = config_get_int_value(config,"TIEMPO_MAXIMO_BLOQUEADO");

    cpu_dispatch = iniciarServidor(ip, puerto_dispatch, logger);
    log_info(logger, "CPU listo para recibir un kernel");
    cliente_dispatch = esperarCliente(cpu_dispatch,logger);

    cpuInterrupt = iniciarServidor(ip, puerto_interrupt, logger);
 	//conexionMemoria = crearConexion(ipMemoria, puertoMemoria, "Memoria");
	//log_info(logger, "Te conectaste con Memoria");
    //int memoria_fd = esperar_memoria(cpuDispatch); Esto es para cuando me conecte con la memoria

	while(cliente_dispatch!=-1) {
		op_code cod_op = recibirOperacion(cliente_dispatch);
		switch (cod_op) {
			case MENSAJE:
				recibirMensaje(cliente_dispatch, logger);
				break;
			case PCB:
				log_info(logger,"Recibi un PCB");
				pcb = recibirPCB(cliente_dispatch);
				loggearPCB(pcb);
				comenzar_ciclo_instruccion();
			   break;
			case -1:
				log_info(logger, "El cliente se desconecto.");
				cliente_dispatch=-1;
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


//--------Ciclo de instruccion---------
void comenzar_ciclo_instruccion(){

	//time_t cronometro = time(NULL);
	op_code proceso_respuesta = CONTINUA_PROCESO;
	operando operador = 0;

	while(proceso_respuesta == CONTINUA_PROCESO){
		t_instruccion* instruccion = fase_fetch();
		int requiero_operador = fase_decode(instruccion);

		if(requiero_operador) {
			operador = fase_fetch_operand(instruccion->parametros[1]);
		}

		proceso_respuesta = fase_execute(instruccion, operador);
		//cronometro = cronometro - time(NULL);
		//estimar_proxima_rafaga(cronometro);
/*
		if(proceso_respuesta == CONTINUA_PROCESO) {
			atender_interrupciones();
		}
*/
	}

}

t_instruccion* fase_fetch(){
	t_instruccion* instruccion = list_get(pcb->listaInstrucciones, pcb->programCounter);
	pcb-> programCounter++;
	return instruccion;
}

int fase_decode(t_instruccion* instruccion){
	return ((instruccion->codigo_operacion) == COPY);
}

operando fase_fetch_operand(operando direccion_operador_a_buscar) {
	//Deberia acceder a la memoria para traerme el operador
	return direccion_operador_a_buscar;
}

op_code fase_execute(t_instruccion* instruccion, uint32_t operador){
	op_code proceso_respuesta = CONTINUA_PROCESO;
	switch(instruccion->codigo_operacion){
		case NO_OP:
			proceso_respuesta = CONTINUA_PROCESO;
			operacion_NO_OP();
			break;
		case IO:
			proceso_respuesta = BLOQUEAR_PROCESO;
			operacion_IO(proceso_respuesta, instruccion->parametros[0]);
			break;
		case READ:
			//Provisorio
			proceso_respuesta = CONTINUA_PROCESO;
			log_info(logger,"Ejecutando READ");
			break;
		case WRITE:
			//Provisorio
			proceso_respuesta = CONTINUA_PROCESO;
			log_info(logger,"Ejecutando WRITE");
			break;
		case COPY:
			//Provisorio
			proceso_respuesta = CONTINUA_PROCESO;
			log_info(logger,"Ejecutando COPY");
			break;
		case EXIT:
			proceso_respuesta = TERMINAR_PROCESO;
			operacion_EXIT(proceso_respuesta);
			break;
	}
	return proceso_respuesta;
}

void operacion_NO_OP(){
	int retardo_noop_microsegundos = 1000 * retardo_noop;
	log_info(logger,"Ejecutando NO_OP: %d",retardo_noop_microsegundos);
	usleep(retardo_noop_microsegundos);
}

void operacion_IO(op_code proceso_respuesta, operando tiempo_bloqueo){
	log_info(logger,"Ejecutando I/O: %d",tiempo_bloqueo);
	t_paquete* paquete = crearPaquete();
	paquete->codigo_operacion = proceso_respuesta;
	preparar_pcb_respuesta(paquete);
	agregarEntero(paquete,tiempo_bloqueo);
	enviarPaquete(paquete,cliente_dispatch);
	eliminarPaquete(paquete);
}

void operacion_EXIT(op_code proceso_respuesta){
	log_info(logger,"Ejecutando EXIT");
	t_paquete* paquete = crearPaquete();
	paquete->codigo_operacion = proceso_respuesta;
	preparar_pcb_respuesta(paquete);
	enviarPaquete(paquete,cliente_dispatch);
	eliminarPaquete(paquete);
}

void preparar_pcb_respuesta(t_paquete* paquete){
	agregarEntero(paquete, pcb->idProceso);
	agregarEntero(paquete, pcb->tamanioProceso);
	agregarEntero(paquete, pcb->programCounter);
	agregarEntero(paquete, pcb->tablaPaginas); //por ahora la tabla de paginas es un entero
	agregarEntero(paquete, pcb->estimacionRafaga);
	agregarEntero(paquete, pcb->duracionUltimaRafaga);
	agregarListaInstrucciones(paquete, pcb->listaInstrucciones);
}

//-----------Ciclo de interrupcion-----------

void atender_interrupciones() {
	log_info(logger,"Entro en atender_interrupciones");
	if(cpuInterrupt != -1) {
		op_code cod_op = recibirOperacion(cpuInterrupt);
		t_pcb* pcbNuevo;

		switch (cod_op) {
			case DESALOJAR_PROCESO:
				pcbNuevo = recibirPCB(cpuInterrupt);
				log_info(logger,"Recibi nuevo PCB");
				enviarPCB(cpuInterrupt, pcb, PCB);
				log_info(logger, "Envio PCB que estaba ejecutando");
				pcb = pcbNuevo;
			   break;
			case -1:
				log_info(logger, "El Kernel no envio ninguna interrupcion");
				cliente_dispatch=-1;
				break;
			default:
				log_warning(logger,"Operacion desconocida.");
				break;
		}
	}
}

void loggearPCB(t_pcb* pcb){
	log_info(logger, "PCB:");
	log_info(logger, "ID: %zu",pcb->idProceso);
}


