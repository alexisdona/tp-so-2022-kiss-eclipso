
#include <netdb.h>
#include "headers/sharedUtils.h"

t_config* iniciarConfig(char* file) {
    t_config* nuevo_config;

    if((nuevo_config = config_create(file)) == NULL) {
        perror("No se pudo leer la configuracion: ");
        exit(-1);
    }
    return nuevo_config;

}

t_log* iniciarLogger(char* file, char* logName) {

    t_log* nuevo_logger = log_create(file, logName, false, LOG_LEVEL_INFO );
    return nuevo_logger;

}

t_paquete* crearPaquete(void)
{
    t_paquete* paquete = malloc(sizeof(t_paquete));
    crearBuffer(paquete);
    return paquete;

}

void crearBuffer(t_paquete* paquete)
{
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 0;
    paquete->buffer->stream = NULL;

}

void* serializarPaquete(t_paquete* paquete, int bytes)
{
    void * magic = malloc(bytes);
    int desplazamiento = 0;

    memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
    desplazamiento+= sizeof(int);
    memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
    desplazamiento+= sizeof(int);
    memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
    desplazamiento+= paquete->buffer->size;

    return magic;
}


void eliminarPaquete(t_paquete* paquete)
{
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

void enviarMensaje(char* mensaje, int fd) {

    t_paquete *paquete = crearPaquete();
    paquete->codigo_operacion = MENSAJE;
    paquete->buffer->size = strlen(mensaje) + 1;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

    int bytes = paquete->buffer->size + 2 * sizeof(int);

    void *a_enviar = serializarPaquete(paquete, bytes);

    send(fd, a_enviar, bytes, 0);

    free(a_enviar);
    eliminarPaquete(paquete);
}

int enviarPaquete(t_paquete* paquete, int socket_consola)
{
    int tamanioCodigoOperacion = sizeof(op_code);
    int tamanioStream = paquete->buffer->size;
    size_t tamanioPayload = sizeof(size_t);


    size_t tamanioPaquete = tamanioCodigoOperacion + tamanioStream + tamanioPayload;
    void* a_enviar = serializarPaquete(paquete, tamanioPaquete);
    printf("paquete->codigo_operacion: %u",paquete->codigo_operacion);
    if(send(socket_consola, a_enviar, tamanioPaquete, 0) == -1){
        perror("Hubo un error enviando la lista de instrucciones: ");
        free(a_enviar);
        return EXIT_FAILURE;
    }

    free(a_enviar);
    return EXIT_SUCCESS;
}

void liberarConexion(int fd)
{
    close(fd);
}

void terminarPrograma(uint32_t conexion, t_log* logger, t_config* config) {

    log_info(logger, "Consola: Terminando programa...");
    log_destroy(logger);
    if(config!=NULL) {
        config_destroy(config);
    }
    liberarConexion(conexion);
}

void recibirMensaje(int socket_cliente, t_log* logger)
{
    int size;
    char* buffer = recibirBuffer((size_t) &size, socket_cliente);
    log_info(logger, "Me llego el mensaje %s", buffer);
    free(buffer);
}

void* recibirBuffer(size_t size, int socket_cliente)
{
    void * buffer;
    buffer = malloc(size);
    recv(socket_cliente, buffer, size, MSG_WAITALL);

    return buffer;
}

void verificarListen(int socket) {
    if (listen(socket, SOMAXCONN) == -1) {
        perror("Hubo un error en el listen: ");
        close(socket);
        exit(-1);
    }
}

void verificarBind(int socket_kernel,  struct addrinfo *kernelinfo) {
    if( bind(socket_kernel, kernelinfo->ai_addr, kernelinfo->ai_addrlen) == -1) {
        perror("Hubo un error en el bind: ");
        close(socket_kernel);
        exit(-1);
    }
}

int esperar_cliente(int socket_servidor, char* nombre_cliente, t_log* logger) {
	// Aceptamos un cliente
    int socket_cliente = accept(socket_servidor, NULL, NULL);

	if (socket_cliente == -1) {
	    perror(strcat("Hubo un error en aceptar una conexión del cliente ", nombre_cliente));
	    close(socket_servidor);
	    exit(-1);
	}

	log_info(logger, strcat("Se conectó el cliente ", nombre_cliente));
	return socket_cliente;
}