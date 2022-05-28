#include "utils.h"


void verificarConnect(int socket_consola, struct sockaddr_in *direccionKernel);

int crear_conexion(char* ip, int puerto)
{
	int socket_consola = socket(AF_INET, SOCK_STREAM, 0);

	if (socket_consola == -1) {
	    perror("Hubo un error al crear el socket de la consola: ");
	    exit(-1);
	}

	struct sockaddr_in direccionKernel;
	direccionKernel.sin_family = AF_INET;
	direccionKernel.sin_addr.s_addr = inet_addr(ip);
	direccionKernel.sin_port = htons(puerto);
	memset(&(direccionKernel.sin_zero),'\0',8); //se rellena con ceros para que tenga el mismo tamaño que socketaddr

    verificarConnect(socket_consola, &direccionKernel);

    return socket_consola;
}

void verificarConnect(int socket_consola, struct sockaddr_in *direccionKernel) {
    if (connect(socket_consola, (void*) direccionKernel, sizeof((*direccionKernel))) == -1) {
        perror("Hubo un problema conectando al servidor: ");
        close(socket_consola);
        exit(-1);
    }
}


void agregarTamanioProceso(t_paquete* paquete, int tamanioProceso) {

    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(tamanioProceso));

    memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanioProceso,
           sizeof(tamanioProceso));

    paquete->buffer->size += sizeof(tamanioProceso);
}

    void agregarInstruccion(t_paquete* paqueteInstrucciones, void* instruccion){
    size_t tamanioOperandos = sizeof(operando)*2;
    int tamanio = sizeof(instr_code)+tamanioOperandos;
    paqueteInstrucciones->buffer->stream =
            realloc(paqueteInstrucciones->buffer->stream, paqueteInstrucciones->buffer->size + tamanio + sizeof(int));

	memcpy(paqueteInstrucciones->buffer->stream + paqueteInstrucciones->buffer->size, instruccion, sizeof(instr_code));
	memcpy(paqueteInstrucciones->buffer->stream + paqueteInstrucciones->buffer->size + sizeof(instr_code), instruccion + sizeof(instr_code), tamanioOperandos);

    paqueteInstrucciones->buffer->size += tamanio;

}





