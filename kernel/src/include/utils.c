

#include "utils.h"


//void deserializarListaInstrucciones(void*, t_paquete*);

t_list* deserializarListaInstrucciones(void *pVoid, size_t tamanioListaInstrucciones, t_list *instrucciones);

int iniciar_kernel(void)
{
	int socket_kernel;

	struct addrinfo hints, *kernelinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(IP, PUERTO, &hints, &kernelinfo);

	// Creamos el socket de escucha del kernel
	socket_kernel = socket(kernelinfo->ai_family,kernelinfo->ai_socktype,kernelinfo->ai_protocol);
	// Asociamos el socket a un puerto
    verificarBind(socket_kernel, kernelinfo);
    // Escuchamos las conexiones entrantes
    verificarListen(socket_kernel);

    freeaddrinfo(kernelinfo);
	log_trace(logger, "Listo para escuchar a mi consola");

	return socket_kernel;
}

void verificarListen(int socket_kernel) {
    if (listen(socket_kernel, SOMAXCONN) == -1) {
        perror("Hubo un error en el listen: ");
        close(socket_kernel);
        exit(-1);
    }
}

void verificarBind(int socket_kernel, const struct addrinfo *kernelinfo) {
    if( bind(socket_kernel, kernelinfo->ai_addr, kernelinfo->ai_addrlen) == -1) {
        perror("Hubo un error en el bind: ");
        close(socket_kernel);
        exit(-1);
    } }

int esperar_consola(int socket_kernel)
{
	// Aceptamos un nuevo consola
    int socket_consola = accept(socket_kernel, NULL, NULL);

	if (socket_consola == -1) {
	    perror("Hubo un error en aceptar una conexión de la consola: ");
	    close(socket_kernel);
	    exit(-1);
	}

	log_info(logger, "Se conecto una consola!");
	return socket_consola;
}

op_code recibirOperacion(int socket_consola)
{
	op_code cod_op;

	if(recv(socket_consola, &cod_op, sizeof(op_code), MSG_WAITALL) > 0) {
        printf("recibirOperacion --> cod_op: %d\n", cod_op);
		return cod_op;
	}
	else
	{
		close(socket_consola);
		return -1;
	}

}

size_t recibirTamanioStream(int socket_consola) {
    size_t tamanioStream;
    void* stream;

    if(recv(socket_consola, stream, sizeof(size_t), 0) > 0) {
        memcpy(&tamanioStream, stream, sizeof(size_t));
        return tamanioStream;
    }
    else
    {
        close(socket_consola);
        return -1;
    }
}

void* recibirBuffer(size_t size, int socket_consola)
{
	void * buffer;
	buffer = malloc(size);
	recv(socket_consola, buffer, size, MSG_WAITALL);

	return buffer;
}

void recibirMensaje(int socket_consola)
{
	int size;
	char* buffer = recibirBuffer(size, socket_consola);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}
/*
bool recibirListaInstrucciones2(int socket_consola, t_list* listaInstrucciones) {
    size_t sizePayload;

    if(recv(socket_consola, &sizePayload, sizeof(size_t), 0) != sizeof(size_t)){
        return false;
    }
    void* stream = malloc(sizePayload);
    if(recv(socket_consola, stream, sizePayload, 0) != sizePayload ) {
        free(stream);
        return false;
    }
    deserializarListaInstrucciones(stream, listaInstrucciones);
}
*/


t_list* recibirListaInstrucciones(int socket_consola) {
    printf("entra en recibirListaInstrucciones");
    size_t tamanoListaInstrucciones;
    t_list * listaInstrucciones = list_create();
    recv(socket_consola, &tamanoListaInstrucciones, sizeof(size_t), 0);
    void *stream = malloc(tamanoListaInstrucciones);
    recv(socket_consola, stream, tamanoListaInstrucciones, 0);

    listaInstrucciones = deserializarListaInstrucciones(stream, tamanoListaInstrucciones, listaInstrucciones);
    return listaInstrucciones;
}

t_list* deserializarListaInstrucciones(void* stream, size_t tamanioListaInstrucciones, t_list* listaInstrucciones) {
    int desplazamiento = 0;
    char** lineas = string_array_new();
    size_t tamanioInstruccion = sizeof(instr_code)+sizeof(operando)*2;
    t_list *valores = list_create();
    while(desplazamiento < tamanioListaInstrucciones) {
		char* valor = malloc(tamanioInstruccion);
		memcpy(valor, stream+desplazamiento, tamanioInstruccion);
		desplazamiento += tamanioInstruccion;
		list_add(valores, valor);
	}
    return valores;
}
/*
void deserializarListaInstrucciones(void* stream, t_paquete* listaInstrucciones) {
    size_t sizeListaInstrucciones;
    memcpy(sizeListaInstrucciones,stream,sizeof);
}
*/
