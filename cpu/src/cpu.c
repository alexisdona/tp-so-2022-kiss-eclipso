#include "cpu.h"
t_log* logger;
int cpuDispatch;
int cpuInterrupt;
t_config * config;
int conexionMemoria;
int clienteDispatch, clienteInterrupt;
t_pcb* pcb;

t_list* interrupciones;

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
//	int memoria_fd = esperar_memoria(cpuDispatch); Esto es para cuando me conecte con la memoria

	while(clienteDispatch!=-1) {
		op_code cod_op = recibirOperacion(clienteDispatch);
		switch (cod_op) {
			case MENSAJE:
                recibirMensaje(clienteDispatch, logger);
		        break;
		    case PCB:
		    	pcb = recibirPCB(clienteDispatch);
		    	comenzarCiclo();
		    	//send()
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

//-----------Serializacion del PCB------------------


t_pcb* recibirPCB(int socketKernel) {
    t_pcb * pcb;
    int tamanioProceso = sizeof(int);
    size_t tamanioTotalStream;
    size_t tamanioPCB;


    recv(socketKernel, &tamanioTotalStream, sizeof(size_t), 0);


    tamanioPCB = tamanioTotalStream - tamanioProceso;

    void *stream = malloc(tamanioPCB);
    recv(socketKernel, stream, tamanioPCB, 0); //le pido la cantidad de bytes que ocupa el pcb
    pcb = deserializarPCB(stream, tamanioPCB, pcb);
    return pcb;
}

/*

t_pcb* deserializarPCB(void* stream, size_t tamanioPcb, t_pcb* pcb) {
    int desplazamiento = 0;

    size_t tamanio =

    //size_t tamanioPCB = sizeof(instr_code)+sizeof(operando)*2; -> modificar a lo nuevo

    t_list *valores = list_create();
    while(desplazamiento < tamanioPcb) {
		char* valor = malloc(tamanio);
		memcpy(valor, stream+desplazamiento, tamanioInstruccion);
		desplazamiento += tamanioInstruccion;
		list_add(valores, valor);
	}
    return valores;
}

void* buffer_deserializar(t_buffer* buffer, size_t tamanio)
{
	if(buffer->tamanio < buffer->desplazamiento + tamanio)
		exit(-2);

	void* datos = malloc(tamanio);
	memcpy(datos, buffer->stream + buffer->desplazamiento, tamanio);
	buffer->desplazamiento += tamanio;

	return datos;
}

*/

//--------Ciclo de instruccion---------


void comenzarCiclo(){

	bool continuar = true;

	while(continuar){
		t_instruccion instruccion = fetch();
			int fetchOperandIfNecessary = decode(instruccion);
			uint32_t operand = fetchOperands(fetchOperandIfNecessary, instruccion);
			execute(instruccion, operand,  pcb);
			continuar = cicloInterrupciones();
	}

}

t_instruccion fetch(){
	t_instruccion instruccion = list_get(pcb->listaInstrucciones, pcb->programCounter);

	pcb-> programCounter++;

	return instruccion;
}

//interpretar qué instrucción es la que se va a ejecutar. Esto es importante para determinar si la próxima etapa (Fetch Operands) es necesaria.
//Siendo específicos, solamente la instrucción COPY tiene operandos que deben ser buscados en memoria antes de poder ejecutarla.

int decode(t_instruccion instruccion) {

	if(instruccion->codigo_operacion == COPY){
		return 1;
	}

	return 0;
}

//En continuación del párrafo anterior, el segundo parámetro de las instrucción COPY representa la dirección lógica del valor que queremos
//escribir, por lo que en esta etapa deberá buscarse dicho valor en memoria antes de seguir a la próxima etapa del ciclo de instrucción.

int fetchOperands(uint32_t fetchOperandIsNecessary, t_instruccion instruccion) {
	return 0;

}

int execute(t_instruccion instruccion, uint32_t operand, t_pcb* pcb){

	//t_pcb* pcb =((t_operacion_servidor) dictionary_int_get(servidor->diccionario_operaciones, paquete->codigo_operacion))(datos);

}

void operacionNoOp(uint32_t operand){

	uint32_t retardoNoOp = config_get_string_value(config,"RETARDO_NOOP");
	wait(operand* retardoNoOp);
}

void operacionIo(uint32_t operand){
	//actualizarPcb
	//agregarInterrupcion DESALOJAR_PROCESO a la lista de interrupciones
}

void operacionExit(){
	//actualizarPcb a Exit
		//agregarInterrupcion DESALOJAR_PROCESO a la lista de interrupciones
}




//-----------Ciclo de interrupcion-----------





