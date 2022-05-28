#include "utils.h"

//void deserializarListaInstrucciones(void*, t_paquete*);

t_list* deserializarListaInstrucciones(void *pVoid, size_t tamanioListaInstrucciones, t_list *instrucciones);

int iniciar_kernel(char* ip, char* puerto)
{
	int socketKernel;

	struct addrinfo hints, *kernelInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip,puerto, &hints, &kernelInfo);

	// Creamos el socket de escucha del kernel
	socketKernel = socket(kernelInfo->ai_family, kernelInfo->ai_socktype, kernelInfo->ai_protocol);
	// Asociamos el socket a un puerto
    verificarBind(socketKernel, kernelInfo);
    // Escuchamos las conexiones entrantes
    verificarListen(socketKernel);

    freeaddrinfo(kernelInfo);
	log_trace(logger, "Listo para escuchar a mi consola");

	return socketKernel;
}

int esperarConsola(int socketKernel)
{
	// Aceptamos un nuevo consola
    int socketConsola = accept(socketKernel, NULL, NULL);

	if (socketConsola == -1) {
	    perror("Hubo un error en aceptar una conexión de la consola: ");
	    close(socketKernel);
	    exit(-1);
	}

	log_info(logger, "Se conecto una consola!");
	return socketConsola;
}

op_code recibirOperacion(int socketConsola)
{
	op_code cod_op;

	if(recv(socketConsola, &cod_op, sizeof(op_code), MSG_WAITALL) > 0) {
        printf("recibirOperacion --> cod_op: %d\n", cod_op);
		return cod_op;
	}
	else
	{
		close(socketConsola);
		return -1;
	}

}

size_t recibirTamanioStream(int socketConsola) {
    size_t tamanioStream;
    void* stream;

    if(recv(socketConsola, stream, sizeof(size_t), 0) > 0) {
        memcpy(&tamanioStream, stream, sizeof(size_t));
        return tamanioStream;
    }
    else
    {
        close(socketConsola);
        return -1;
    }
}

int recibirTamanioProceso(int socketConsola) {
    int tamanioProceso;
    recv(socketConsola, &tamanioProceso, sizeof(int), 0);
    return tamanioProceso;
}

t_list* recibirListaInstrucciones(int socketConsola) {
    t_list * listaInstrucciones = list_create();
    int tamanioProceso = sizeof(int);
    size_t tamanioTotalStream;
    size_t tamanioListaInstrucciones;
    recv(socketConsola, &tamanioTotalStream, sizeof(size_t), 0);
    tamanioListaInstrucciones = tamanioTotalStream - tamanioProceso;

    void *stream = malloc(tamanioListaInstrucciones);
    recv(socketConsola, stream, tamanioListaInstrucciones, 0); //le pido la cantidad de bytes que ocupa la lista de instrucciones nada mas
    listaInstrucciones = deserializarListaInstrucciones(stream, tamanioListaInstrucciones, listaInstrucciones);
    return listaInstrucciones;
}

t_list* deserializarListaInstrucciones(void* stream, size_t tamanioListaInstrucciones, t_list* listaInstrucciones) {
    int desplazamiento = 0;
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

