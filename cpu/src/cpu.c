#include "cpu.h"
#include <math.h>

t_log* logger;
int cpu_interrupt;
t_config * config;
int conexionMemoria;
int cliente_dispatch;
t_pcb* pcb;
int retardo_noop;
int cpu_dispatch;
int tamanio_pagina, entradas_por_tabla;
t_list* tlb;
char* algoritmo_reemplazo_tlb;
uint32_t entradas_max_tlb;
char* ip, *ip_memoria, *puerto_dispatch, *puerto_interrupt;
int puerto_memoria;
int tiempo_bloqueo;
int hay_interrupcion; //variable global que se llena cuando  hay interrupción del kernel

void levantar_configs();


int main(void) {

	logger = iniciarLogger(LOG_FILE,"CPU");
	config = iniciarConfig(CONFIG_FILE);
	tlb = list_create();
    levantar_configs();

    cpu_dispatch = iniciarServidor(ip, puerto_dispatch, logger);

    log_info(logger, "CPU listo para recibir un kernel");

    cpu_interrupt = iniciarServidor(ip, puerto_interrupt, logger);

    conexionMemoria = crearConexion(ip_memoria, puerto_memoria, "Memoria");
    handshake_memoria(conexionMemoria);
    enviarMensaje("Hola MEMORIA soy el CPU", conexionMemoria);

    while (1) {
        escuchar_cliente();
    }


}

void levantar_configs() {
    ip = config_get_string_value(config, "IP_CPU");
    ip_memoria = config_get_string_value(config,"IP_MEMORIA");

    puerto_memoria = config_get_int_value(config,"PUERTO_MEMORIA");
    puerto_dispatch = config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
    puerto_interrupt = config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");
    retardo_noop = config_get_int_value(config,"RETARDO_NOOP");
    tiempo_bloqueo = config_get_int_value(config,"TIEMPO_MAXIMO_BLOQUEADO");
    algoritmo_reemplazo_tlb = config_get_string_value(config, "REEMPLAZO_TLB");
    entradas_max_tlb = config_get_int_value(config, "ENTRADAS_TLB");
}

//--------Ciclo de instruccion---------
void comenzar_ciclo_instruccion(){

	op_code proceso_respuesta = CONTINUA_PROCESO;
	operando operador = 0;

	while(proceso_respuesta == CONTINUA_PROCESO){
		t_instruccion* instruccion = fase_fetch();
		int requiero_operador = fase_decode(instruccion);

		if(requiero_operador) {
			operador = fase_fetch_operand(instruccion->parametros[1]);
		}

		proceso_respuesta = fase_execute(instruccion, operador);

        if(hay_interrupcion>0) {
            atender_interrupcion();
        }

	}



}

t_instruccion* fase_fetch(){
	t_instruccion* instruccion = list_get(pcb->listaInstrucciones, pcb->programCounter);
	pcb->programCounter++;
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
            log_info(logger,"Ejecutando READ");
			proceso_respuesta = CONTINUA_PROCESO;
			operacion_READ(instruccion->parametros[0]);

			break;
		case WRITE:
            log_info(logger,"Ejecutando WRITE");
            proceso_respuesta = CONTINUA_PROCESO;
            operacion_WRITE(instruccion->parametros[0], instruccion->parametros[1]);
			break;
		case COPY:
            log_info(logger,"Ejecutando COPY");
            proceso_respuesta = CONTINUA_PROCESO;
			operacion_COPY(instruccion->parametros[0], instruccion->parametros[1]);
			break;
		case EXIT:
			proceso_respuesta = TERMINAR_PROCESO;
			operacion_EXIT(proceso_respuesta);
			list_clean(tlb); //se limpia la tlb para usar en el próximo proceso
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
    enviarPCB(cliente_dispatch, pcb,  proceso_respuesta);
    logear_PCB(logger,pcb,"ENVIADO");
}

void operacion_EXIT(op_code proceso_respuesta){
	log_info(logger,"Ejecutando EXIT");
    enviarPCB(cliente_dispatch, pcb, proceso_respuesta);
    logear_PCB(logger,pcb,"ENVIADO");
    pcb=NULL;
}

void operacion_READ(operando dirLogica){

	dir_fisica* dir_fisica = obtener_direccion_fisica(dirLogica);
    uint32_t valor = leer_en_memoria(dir_fisica);
}

void operacion_COPY(uint32_t direccion_logica_destino, uint32_t direccion_logica_origen){
    dir_fisica* dir_fisica_destino = obtener_direccion_fisica(direccion_logica_destino);
    dir_fisica* dir_fisica_origen =  obtener_direccion_fisica(direccion_logica_origen);
    uint32_t valor_en_origen = leer_en_memoria(dir_fisica_origen);
    escribir_en_memoria(dir_fisica_destino, valor_en_origen);

}

void operacion_WRITE(uint32_t direccion_logica, uint32_t valor){
    dir_fisica* dir_fisica = obtener_direccion_fisica(direccion_logica);
    escribir_en_memoria(dir_fisica, valor);
}

//-----------Ciclo de interrupcion-----------

void atender_interrupcion() {
    log_info(logger, "El CPU esta atendiendo una interrupcion...");
	log_info(logger,"DESALOJANDO PROCESO...");
	log_info(logger,"DISPATCH: %d",cliente_dispatch);
	enviarPCB(cliente_dispatch, pcb, DESALOJAR_PROCESO);
	logear_PCB(logger, pcb,"ENVIADO");
	log_info(logger, "Se envia la PCB que se estaba ejecutando...");
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
            marco = obtener_marco_memoria(tabla_segundo_nivel, entrada_tabla_2do_nivel, numero_pagina);
            tlb_actualizar(numero_pagina, marco);
        }
        dir_fisica * direccion_fisica = malloc(sizeof(dir_fisica));
        direccion_fisica->numero_pagina = numero_pagina;
        direccion_fisica->marco = marco;
        direccion_fisica->desplazamiento = desplazamiento;
        return direccion_fisica;
    }

    else {
        log_error(logger, "El proceso intento acceder a una direccion logica invalida");
        //operacion_EXIT() ? Se lo tiene que matar si intenta acceder a una direccion no valida?
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

uint32_t obtener_marco_memoria(uint32_t nro_tabla_segundo_nivel, uint32_t entrada_tabla_2do_nivel, uint32_t numero_pagina) {
    t_paquete * paquete = crearPaquete();
    paquete->codigo_operacion = OBTENER_MARCO;
    agregarEntero(paquete, pcb->idProceso);
    agregarEntero4bytes(paquete, nro_tabla_segundo_nivel);
    agregarEntero4bytes(paquete, entrada_tabla_2do_nivel);
    agregarEntero4bytes(paquete, numero_pagina);
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
                entrada_tlb->veces_referenciada+=1;
                return entrada_tlb->marco;
            }
        }
    }
    return -1;
}
void reemplazar_entrada_tlb(tlb_entrada* entrada) {

    if (string_equals_ignore_case(algoritmo_reemplazo_tlb, "FIFO")){
        list_remove(tlb, 0);
        list_add(tlb, entrada);
    }
    else { //LRU
        list_sort(tlb, comparator);
        list_remove(tlb, 0);
        list_add(tlb, entrada);

    }
}

void tlb_actualizar(uint32_t numero_pagina, uint32_t marco){

	tlb_entrada* tlb_entrada = malloc(sizeof(tlb_entrada));
	tlb_entrada ->marco = marco;
	tlb_entrada ->pagina = numero_pagina;
	tlb_entrada->veces_referenciada=1;

	if(list_size(tlb) >= entradas_max_tlb){
	        reemplazar_entrada_tlb(tlb_entrada);
        }
	else
	{
		list_add(tlb, tlb_entrada);
	}
}

static bool comparator (void* entrada1, void* entrada2) {
    return (((tlb_entrada *) entrada1)->veces_referenciada) < (((tlb_entrada *) entrada2)->veces_referenciada); }


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
                obtuve_valor = 1;
                break;
        }
    }
    return valor_leido;

}


void escribir_en_memoria(dir_fisica * direccion_fisica, uint32_t valor) {
    t_paquete* paquete = crearPaquete();
    paquete->codigo_operacion = ESCRIBIR_MEMORIA;
    agregarEntero(paquete, pcb->idProceso);
    agregarEntero4bytes(paquete, direccion_fisica->numero_pagina); // lo necesito para actualizar el proceso en swap
    agregarEntero4bytes(paquete, direccion_fisica->marco);
    agregarEntero4bytes(paquete, direccion_fisica->desplazamiento);
    agregarEntero4bytes(paquete, valor);
    enviarPaquete(paquete, conexionMemoria);
    eliminarPaquete(paquete);

    int recibi_mensaje = 0;
    while (conexionMemoria != -1 && recibi_mensaje == 0) {
        op_code cod_op = recibirOperacion(conexionMemoria);
        switch(cod_op) {
            case MENSAJE:
                recibirMensaje(conexionMemoria, logger);
                recibi_mensaje = 1;
                break;
        }
    }
}


void escuchar_cliente() {
    hilo_dispatch();
    hilo_interrupt();
    }

void hilo_dispatch() {
    pthread_t hilo_dispatch;
    t_procesar_conexion_attrs* attrs = malloc(sizeof(t_procesar_conexion_attrs));
    attrs->log = logger;
    pthread_create(&hilo_dispatch, NULL, (void*) procesar_conexion_dispatch, (void*) attrs);
    pthread_detach(hilo_dispatch);
}

void hilo_interrupt() {
    pthread_t hilo_interrupt;
    t_procesar_conexion_attrs* attrs = malloc(sizeof(t_procesar_conexion_attrs));
    attrs->log = logger;
    pthread_create(&hilo_interrupt, NULL, (void*) procesar_conexion_interrupt, (void*) attrs);
    pthread_detach(hilo_interrupt);
}



void procesar_conexion_interrupt(void* void_args) {

    t_procesar_conexion_attrs *attrs = (t_procesar_conexion_attrs *) void_args;
    t_log *logger = attrs->log;
    int cliente_interrupt = esperarCliente(cpu_interrupt, logger);
    free(attrs);

    while (cliente_interrupt != -1) {
        op_code cod_op = recibirOperacion(cliente_interrupt);
        switch (cod_op) {

            case INTERRUPCION:
                log_info(logger, "Hubo una interrupción");
                hay_interrupcion += 1;
                break;
        }
    }
}


void procesar_conexion_dispatch(void* void_args) {

    t_procesar_conexion_attrs* attrs = (t_procesar_conexion_attrs*) void_args;
    t_log* logger = attrs->log;
    int cliente_dispatch = esperarCliente(cpu_dispatch, logger);
    free(attrs);

    while(cliente_dispatch != -1) {
        op_code cod_op = recibirOperacion(cliente_dispatch);
        switch (cod_op) {

            case MENSAJE:
                recibirMensaje(cliente_dispatch, logger);
                break;
            case PCB:
                printf("\n");
                log_info(logger,"RECIBI PCB");
                pcb = recibirPCB(cliente_dispatch);
                logear_PCB(logger,pcb,"RECIBIDO");
                list_clean(tlb);
                comenzar_ciclo_instruccion();
                break;

        }

    }
}
