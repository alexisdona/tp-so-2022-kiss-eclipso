#include <consola.h>
#include "utils.h"


void verificarConnect(int socket_consola, struct sockaddr_in *direccionKernel);

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * paqueteSerializado = malloc(bytes);
	int desplazamiento = 0;

	memcpy(paqueteSerializado + desplazamiento, &(paquete->codigo_operacion), sizeof(op_code)); //OP_CODE = LISTA_INSTRUCCIONES
	desplazamiento+= sizeof(op_code);
	memcpy(paqueteSerializado + desplazamiento, &(paquete->buffer->size), sizeof(size_t)); // TAMAÑO DEL STREAM A ENVIAR
	desplazamiento+= sizeof(size_t);
	memcpy(paqueteSerializado + desplazamiento, paquete->buffer->stream, paquete->buffer->size);

	return paqueteSerializado;
}

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

void enviar_mensaje(char* mensaje, int socket_consola)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_consola, a_enviar, bytes, 0);

	free(a_enviar);
    eliminarPaquete(paquete);
}


void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;

}

t_paquete* crear_paquete(void)
{
    t_paquete* paquete = malloc(sizeof(t_paquete));
	//paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;

}

void agregarInstruccion(t_paquete* paqueteInstrucciones, void* instruccion)
{   size_t tamanioOperandos = sizeof(operando)*2;
    int tamanio = sizeof(instr_code)+tamanioOperandos;
    paqueteInstrucciones->buffer->stream =
            realloc(paqueteInstrucciones->buffer->stream, paqueteInstrucciones->buffer->size + tamanio + sizeof(int));

	memcpy(paqueteInstrucciones->buffer->stream + paqueteInstrucciones->buffer->size, instruccion, sizeof(instr_code));
	memcpy(paqueteInstrucciones->buffer->stream + paqueteInstrucciones->buffer->size + sizeof(instr_code), instruccion + sizeof(instr_code), tamanioOperandos);

    paqueteInstrucciones->buffer->size += tamanio;

}

int enviarPaquete(t_paquete* listaInstrucciones, int socket_consola)
{
    int tamanioCodigoOperacion = sizeof(op_code);
	int tamanioStream = listaInstrucciones->buffer->size;
	size_t tamanioPayload = sizeof(size_t);
    size_t tamanioPaquete = tamanioCodigoOperacion + tamanioStream + tamanioPayload;
    void* a_enviar = serializar_paquete(listaInstrucciones, tamanioPaquete);

	if(send(socket_consola, a_enviar, tamanioPaquete, 0) == -1){
	    perror("Hubo un error enviando la lista de instrucciones: ");
	    free(a_enviar);
	    return EXIT_FAILURE;
	}

	free(a_enviar);
	return EXIT_SUCCESS;
}

void eliminarPaquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_consola)
{
	close(socket_consola);
}

