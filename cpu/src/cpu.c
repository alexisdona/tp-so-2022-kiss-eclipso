#include "cpu.h"
#include <math.h>

t_log* logger;
int cpu_dispatch;
int cpuInterrupt;
t_config * config;
int conexionMemoria;
int cliente_dispatch;
t_pcb* pcb;
int retardo_noop;
t_list* interrupciones;
int alpha = 0.5; //Provisorio, debiera ser enviado por el kernel al conectarse una unica vez.
int tamanio_pagina, entradas_por_tabla;
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
 	handshake_memoria(conexionMemoria);
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
			printf("\nejecuto read\n");
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
    enviarPCB(cliente_dispatch, pcb, proceso_respuesta);
}

void operacion_EXIT(op_code proceso_respuesta){
	log_info(logger,"Ejecutando EXIT");
    enviarPCB(cliente_dispatch, pcb, proceso_respuesta);
}

void operacion_READ(operando dirLogica){

	dir_fisica* dir_fisica = obtener_direccion_fisica(dirLogica);
	printf("\ndir_fisica->marco: %d\n", dir_fisica->marco);
    printf("\ndir_fisica->desplazamiento: %d\n", dir_fisica->desplazamiento);
    uint32_t valor = leer_en_memoria(dir_fisica);
    printf("\nCPU --El valor leido en memoria es>  %d\n", valor);
}

void operacion_WRITE(){

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

dir_fisica* obtener_direccion_fisica(uint32_t direccion_logica) {

    if (direccion_logica < pcb->tamanioProceso) {
        uint32_t numero_pagina = floor(direccion_logica / tamanio_pagina);
        uint32_t entrada_tabla_1er_nivel = floor(numero_pagina / entradas_por_tabla);
        uint32_t entrada_tabla_2do_nivel = numero_pagina % entradas_por_tabla;
        uint32_t desplazamiento = direccion_logica - (numero_pagina * tamanio_pagina);

        uint32_t marco;
        marco = tlb_obtener_marco(numero_pagina);
        if (marco == -1 ) {
         //TLB_MISS
            uint32_t tabla_segundo_nivel = obtener_tabla_segundo_nivel(pcb->tablaPaginas, entrada_tabla_1er_nivel);
            marco = obtener_marco_memoria(tabla_segundo_nivel, entrada_tabla_2do_nivel);
            tlb_actualizar(numero_pagina, marco);
        }
        printf("\nMARCO = %d\n", marco);
        dir_fisica * direccion_fisica = malloc(sizeof(dir_fisica));
        direccion_fisica->marco = marco;
        direccion_fisica->desplazamiento = desplazamiento;
        return direccion_fisica;
    }

    else {
        log_error(logger, "El proceso intento acceder a una direccion logica invalida");
        return EXIT_FAILURE;
    }
}

uint32_t obtener_tabla_segundo_nivel(size_t tabla_paginas, uint32_t entrada_tabla_1er_nivel) {
// primer acceso a memoria para obtener la entrada de la tabla de segundo nivel
    t_paquete * paquete = crearPaquete();
    paquete->codigo_operacion = OBTENER_ENTRADA_SEGUNDO_NIVEL;
    agregarEntero(paquete, tabla_paginas);
    agregarEntero4bytes(paquete, entrada_tabla_1er_nivel);
    enviarPaquete(paquete, conexionMemoria);
    uint32_t entrada_segundo_nivel;
    //obtener entrada de tabla de segundo nivel
    int obtuve_valor_tabla = 0;
    while (conexionMemoria != -1 && obtuve_valor_tabla == 0) {
            op_code cod_op = recibirOperacion(conexionMemoria);
            switch(cod_op) {
                case OBTENER_ENTRADA_SEGUNDO_NIVEL:
                    ;
                    void* buffer = recibirBuffer(conexionMemoria);
                    memcpy(&entrada_segundo_nivel, buffer, sizeof(uint32_t));
                    printf("\nentrada_tabla_segundo_nivel: %d\n", entrada_segundo_nivel);
                    obtuve_valor_tabla = 1;
                    break;
            }
        }
        return entrada_segundo_nivel;
    }

uint32_t obtener_marco_memoria(uint32_t nro_tabla_segundo_nivel, uint32_t entrada_tabla_2do_nivel) {
    t_paquete * paquete = crearPaquete();
    paquete->codigo_operacion = OBTENER_MARCO;
    agregarEntero4bytes(paquete, nro_tabla_segundo_nivel);
    agregarEntero4bytes(paquete, entrada_tabla_2do_nivel);
    enviarPaquete(paquete, conexionMemoria);
    eliminarPaquete(paquete);
    uint32_t marco;

    int obtuve_marco = 0;
    while (conexionMemoria != -1 && obtuve_marco == 0) {
        op_code cod_op = recibirOperacion(conexionMemoria);
        switch(cod_op) {
            case OBTENER_MARCO:
                ;
                void* buffer = recibirBuffer(conexionMemoria);
                memcpy(&marco, buffer, sizeof(uint32_t));
                printf("\nmarco de memoria: %d\n", marco);
                obtuve_marco = 1;
                break;
        }
    }
    return marco;

}


////--------------------------------------------------------TLB------------------------------------------------------------------

uint32_t tlb_obtener_marco(uint32_t numero_pagina) {
    tlb_entrada * entrada_tlb;
    if (list_size(tlb) > 0) {
        for (int i=0; i < list_size(tlb); i++) {
            entrada_tlb = list_get(tlb,i);
            if (entrada_tlb->pagina == numero_pagina) {
                return entrada_tlb->marco;
            }
        }
    }
    return -1;
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

void handshake_memoria(int conexionMemoria){
  op_code opCode = recibirOperacion(conexionMemoria);
  size_t tamanio_stream;
  if (opCode==HANDSHAKE_MEMORIA) {
      recv(conexionMemoria, &tamanio_stream, sizeof(size_t), 0); // no me importa en este caso
      recv(conexionMemoria, &tamanio_pagina, sizeof(int), 0);
      recv(conexionMemoria, &entradas_por_tabla, sizeof(int), 0);
  }
}

uint32_t leer_en_memoria(dir_fisica * direccion_fisica) {
    t_paquete* paquete = crearPaquete();
    paquete->codigo_operacion = LEER_MEMORIA;
    agregarEntero4bytes(paquete, direccion_fisica->marco);
    agregarEntero4bytes(paquete, direccion_fisica->desplazamiento);
    enviarPaquete(paquete, conexionMemoria);
    eliminarPaquete(paquete);
    uint32_t valor_leido;

    int obtuve_valor = 0;
    while (conexionMemoria != -1 && obtuve_valor == 0) {
        op_code cod_op = recibirOperacion(conexionMemoria);
        switch(cod_op) {
            case LEER_MEMORIA:
                ;
                void* buffer = recibirBuffer(conexionMemoria);
                memcpy(&valor_leido, buffer, sizeof(uint32_t));
                printf("\nvalor leido de memoria: %d\n", valor_leido);
                obtuve_valor = 1;
                break;
        }
    }
    return valor_leido;

}








