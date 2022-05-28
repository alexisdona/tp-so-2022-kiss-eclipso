#include "utils.h"


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





