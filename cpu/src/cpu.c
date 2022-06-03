#include "cpu.h"

t_log* logger;
int cpuDispatch;
int cpuInterrupt;
t_config * config;
int conexionMemoria;
int clienteDispatch, clienteInterrupt;
t_pcb* pcb;
int retardo_noop;
t_list* interrupciones;

int main(void) {

	logger = iniciarLogger(LOG_FILE,"CPU");
    config = iniciarConfig(CONFIG_FILE);

	char* ip = config_get_string_value(config,"IP_CPU");
	char* ipMemoria = config_get_string_value(config,"IP_MEMORIA");
	int puertoMemoria = config_get_int_value(config,"PUERTO_MEMORIA");
    char* puertoDispatch = config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
    char* puertoInterrupt = config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");
    retardo_noop = config_get_int_value(config,"RETARDO_NOOP");

    cpuDispatch = iniciarServidor(ip, puertoDispatch, logger);
 //   cpuInterrupt = iniciarServidor(ip, puertoInterrupt, logger);

 	conexionMemoria = crearConexion(ipMemoria, puertoMemoria, "Memoria");
	log_info(logger, "Te conectaste con Memoria");

    clienteDispatch = esperarCliente(cpuDispatch,logger);
//	int memoria_fd = esperar_memoria(cpuDispatch); Esto es para cuando me conecte con la memoria

	while(clienteDispatch!=-1) {
		op_code cod_op = recibirOperacion(clienteDispatch);
		switch (cod_op) {
					case MENSAJE:
		                recibirMensaje(clienteDispatch, logger);
				        break;
				    case PCB:
				    	pcb = recibirPCB(clienteDispatch);
				    	comenzar_ciclo_instruccion();
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


//--------Ciclo de instruccion---------
void comenzar_ciclo_instruccion(){

	t_proceso_respuesta* proceso_respuesta;

	proceso_respuesta->estadoProceso = CONTINUA_PROCESO;
	operando operador = 0;

	//PRENDER CRONOMETRO

	while(proceso_respuesta->estadoProceso == CONTINUA_PROCESO){
			t_instruccion* instruccion = fase_fetch();
				int requiero_operador = fase_decode(instruccion);
				if(requiero_operador) {
					operador = fase_fetch_operand(instruccion->parametros[1]);
				}
				proceso_respuesta = fase_execute(instruccion, operador);
				if(proceso_respuesta->estadoProceso == CONTINUA_PROCESO) {
					//ciclo_interrupciones();
				}
				else{

				}
		}


	//TERMINAR CRONOMETRO
	//CAMBIAR EN PCB -> TIEMPO_ESTIMADO

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

t_proceso_respuesta* fase_execute(t_instruccion* instruccion, uint32_t operador){
	t_proceso_respuesta* proceso_respuesta;
	estado_proceso estado_proceso = CONTINUA_PROCESO;
	proceso_respuesta ->pcb = pcb;

	switch(instruccion->codigo_operacion){
		case NO_OP:
			estado_proceso = CONTINUA_PROCESO;
			operacion_NO_OP();
			break;
		case IO:
			proceso_respuesta->estadoProceso = BLOQUEAR_PROCESO;
			operacion_IO(proceso_respuesta, instruccion->parametros[0]);
			break;
		case READ:
			//Provisorio
			estado_proceso = CONTINUA_PROCESO;
			break;
		case WRITE:
			//Provisorio
			estado_proceso = CONTINUA_PROCESO;
			break;
		case COPY:
			//Provisorio
			estado_proceso = CONTINUA_PROCESO;
			break;
		case EXIT:
			proceso_respuesta->estadoProceso = FINALIZAR_PROCESO;
			operacion_EXIT(proceso_respuesta);
			break;
	}

	return proceso_respuesta;
}

void operacion_NO_OP(){
	int retardo_noop_microsegundos = 1000 * retardo_noop;
	usleep(retardo_noop_microsegundos);
}

void operacion_IO(t_proceso_respuesta proceso_respuesta, operando tiempo_bloqueo){

	proceso_respuesta->tiempoBloqueo = tiempo_bloqueo;

}

void operacion_EXIT(t_proceso_respuesta proceso_respuesta){
	t_paquete* paquete = crearPaquete();
	paquete->codigo_operacion = TERMINAR_PROCESO;

	agregarEntero(paquete, pcb->idProceso);
	    agregarEntero(paquete, pcb->tamanioProceso);
	    agregarEntero(paquete, pcb->programCounter);
	    agregarEntero(paquete, pcb->tablaPaginas); //por ahora la tabla de paginas es un entero
	    agregarEntero(paquete, pcb->estimacionRafaga);
	    agregarEntero(paquete, pcb->duracionUltimaRafaga);
	    agregarListaInstrucciones(paquete, pcb->listaInstrucciones);

	    enviarPaquete(paquete,clienteDispatch);
	    eliminarPaquete(paquete);

}


//-----------Ciclo de interrupcion-----------





