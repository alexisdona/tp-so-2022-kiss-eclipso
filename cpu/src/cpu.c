#include "cpu.h"

t_log* logger;
int cpu_dispatch;
int cpuInterrupt;
t_config * config;
int conexionMemoria;
int cliente_dispatch, clienteInterrupt;
t_pcb* pcb;
int retardo_noop;
t_list* interrupciones;
int alpha = 0.5; //Provisorio, debiera ser enviado por el kernel al conectarse una unica vez.
uint32_t cant_entradas_x_tabla_de_pagina;
uint32_t tamanio_pagina;
t_list* tlb;
pthread_mutex_t mutex_algoritmos;
char* algoritmo_reemplazo_tlb;
uint32_t entradas_max_tlb;
uint32_t pid =-1;

void imprimirListaInstrucciones(t_pcb *pcb);

int main(void) {

	logger = iniciarLogger(LOG_FILE,"CPU");
	config = iniciarConfig(CONFIG_FILE);
	tlb = list_create();
	char* ip = config_get_string_value(config,"IP_CPU");
	char* ip_memoria = config_get_string_value(config,"IP_MEMORIA");

	int puerto_memoria = config_get_int_value(config,"PUERTO_MEMORIA");
	char* puerto_dispatch = config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
	char* puerto_interrupt = config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");
	retardo_noop = config_get_int_value(config,"RETARDO_NOOP");
	int tiempo_bloqueo = config_get_int_value(config,"TIEMPO_MAXIMO_BLOQUEADO");

	algoritmo_reemplazo_tlb = config_get_string_value(config, "REEMPLAZO_TLB");
	entradas_max_tlb = config_get_int_value(config, "ENTRADAS_TLB");

	cpu_dispatch = iniciarServidor(ip, puerto_dispatch, logger);
	log_info(logger, "CPU listo para recibir un kernel");
	cliente_dispatch = esperarCliente(cpu_dispatch,logger);

    cpuInterrupt = iniciarServidor(ip, puerto_interrupt, logger);
 	conexionMemoria = crearConexion(ip_memoria, puerto_memoria, "CPU");
 	enviarMensaje("Hola MEMORIA soy el CPU", conexionMemoria);
	//log_info(logger, "Te conectaste con Memoria");
    //int memoria_fd = esperar_memoria(cpuDispatch); Esto es para cuando me conecte con la memoria

	while(cliente_dispatch!=-1) {

		op_code cod_op = recibirOperacion(cliente_dispatch);

		switch (cod_op) {
			case MENSAJE:
				recibirMensaje(cliente_dispatch, logger);
				break;
			case PCB:
			    printf("\n");
				log_info(logger,"Recibi un PCB");
				pcb = recibirPCB(cliente_dispatch);
				limpiar_tlb();
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
			operacion_READ(instruccion->parametros[0]);
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

void operacion_READ(operando dirLogica){

	dir_fisica* dir_fisica = traducir_direccion_logica(dirLogica);

	//Aca pedimos el dato que esta en esa direccion fisica
	// ¿Lo muestra la cpu? o lo mostramos desde memoria y logueamos la operacion

}

void operacion_WRITE(){

}

//------------------------------------------------------------------------

void preparar_pcb_respuesta(t_paquete* paquete){
	agregarEntero(paquete, pcb->idProceso);
	agregarEntero(paquete, pcb->tamanioProceso);
	agregarEntero(paquete, pcb->programCounter);
	agregarEntero(paquete, pcb->tablaPaginas); //por ahora la tabla de paginas es un entero
	agregarEntero(paquete, pcb->estimacionRafaga);
	agregarEntero(paquete, pcb->duracionUltimaRafaga);
	agregarListaInstrucciones(paquete, pcb->listaInstrucciones);
}

void estimar_proxima_rafaga(time_t tiempo){
	int tiempo_cpu = tiempo / 1000;
	pcb->estimacionRafaga = alpha*tiempo_cpu + (1-alpha)*(pcb->estimacionRafaga);
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

//---------------------------------------------------------MMU--------------------------------------------------------


dir_fisica* traducir_direccion_logica(uint32_t dir_logica_data){

	dir_fisica* dir_fisica = malloc(sizeof(dir_fisica));
	uint32_t numero_pagina = (dir_logica_data/tamanio_pagina);
	uint32_t offset = dir_logica_data - numero_pagina * tamanio_pagina;

	uint32_t entrada = tlb_existe(numero_pagina);

	if(entrada >= 0){

		dir_fisica ->marco = tlb_obtener_marco(entrada);
		dir_fisica -> offset = offset;

		if(strcmp(algoritmo_reemplazo_tlb, "LRU")==0)
		{
			tlb_entrada* entrada_aux = list_get(tlb, entrada);
			list_remove(tlb, entrada);
			list_add(tlb, entrada_aux);
		}

		return dir_fisica;

	}else {

		dir_logica* dir_logica = malloc(sizeof(dir_logica));

		dir_logica -> offset = offset;
		dir_logica -> entrada_tabla_primer_nivel = (numero_pagina / cant_entradas_x_tabla_de_pagina);
		dir_logica -> entrada_tabla_segundo_nivel = numero_pagina % cant_entradas_x_tabla_de_pagina;

		uint32_t numero_tabla_segundo_nivel = pedir_a_memoria_num_tabla_segundo_nivel(dir_logica ->entrada_tabla_primer_nivel);

		dir_fisica = malloc(sizeof(dir_fisica));
		dir_fisica -> marco = pedir_a_memoria_marco(numero_tabla_segundo_nivel, dir_logica ->entrada_tabla_segundo_nivel);
		dir_fisica ->offset = dir_logica ->offset;

		tlb_actualizar(numero_pagina, dir_fisica ->marco);
		return dir_fisica;
	}
}

uint32_t pedir_a_memoria_num_tabla_segundo_nivel(uint32_t dato){
	//paso los numeros y el pcb (pcb es variable global)
	return 0;
}

uint32_t pedir_a_memoria_marco(uint32_t dato,uint32_t dato2){
	return 0;
	//paso los numeros y el pcb
}

////--------------------------------------------------------TLB------------------------------------------------------------------

//Retorna la entrada donde se encuentra esa estructura {pagina|marco}
uint32_t tlb_existe(uint32_t numero_pagina){

	tlb_entrada* aux;

	for (int i=0; i < list_size(tlb); i++)
	{
		aux = list_get(tlb, i);
		if (aux->pagina == numero_pagina){
			return i;
		}
	}
	return -1;

}

uint32_t tlb_obtener_marco(uint32_t entrada){

	tlb_entrada* tlb_entrada = list_get(tlb, entrada);

	return tlb_entrada->marco;
}

void tlb_actualizar(uint32_t numero_pagina, uint32_t marco){

	tlb_entrada* tlb_entrada = malloc(sizeof(tlb_entrada));
	tlb_entrada ->marco = marco;
	tlb_entrada ->pagina = numero_pagina;

	if(list_size(tlb) >= entradas_max_tlb){
		//Reemplazo el primer elemento de la lista

		list_remove(tlb, 0);
		list_add(tlb, tlb_entrada);

	}else {
		list_add(tlb, tlb_entrada);
	}
}

void limpiar_tlb(){

	if(pcb->idProceso != pid){
		list_clean(tlb);
		pid = pcb->idProceso;
	}
}










