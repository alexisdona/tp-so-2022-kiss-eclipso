

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

    t_log* nuevo_logger = log_create(file, logName, 1, LOG_LEVEL_DEBUG );
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

void* serializarPaquete(t_paquete* paquete, size_t bytes)
{
    void * magic = malloc(bytes);
    int desplazamiento = 0;

    memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(op_code));
    desplazamiento+= sizeof(op_code);
    memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(size_t));
    desplazamiento+= sizeof(size_t);
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
    /*Se envia el tamaño total del paquete que es el stream más tamaño de codigo de operacion + tamanño de lo que indica size del stream*/
    int bytes = paquete->buffer->size + sizeof(op_code) + sizeof(size_t);

    void *a_enviar = serializarPaquete(paquete, bytes);

    send(fd, a_enviar, bytes, 0);

    free(a_enviar);
    eliminarPaquete(paquete);
}

int enviarPaquete(t_paquete* paquete, int socketCliente)
{
    int tamanioCodigoOperacion = sizeof(op_code);
    int tamanioStream = paquete->buffer->size;
    size_t tamanioPayload = sizeof(size_t);


    size_t tamanioPaquete = tamanioCodigoOperacion + tamanioStream + tamanioPayload;
    void* a_enviar = serializarPaquete(paquete, tamanioPaquete);

    if(send(socketCliente, a_enviar, tamanioPaquete, 0) == -1){
        perror("Hubo un error enviando el paquete: ");
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

void recibirMensaje(int socketCliente, t_log* logger)
{
    char* buffer = recibirBuffer(socketCliente);
    log_info(logger, "Me llego el mensaje %s", buffer);
    free(buffer);
}

void* recibirBuffer(int socketCliente)
{
    void * buffer;
    size_t streamSize;

    //Recibo el tamaño del mensaje
    recv(socketCliente, &streamSize, sizeof(size_t), MSG_WAITALL);
    //malloqueo el tamaño del mensaje y lo recibo en buffer
    buffer = malloc(streamSize);
    recv(socketCliente, buffer, streamSize, MSG_WAITALL);

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

int crearConexion(char* ip, int puerto, char* nombreCliente){ //TODO agregar nombre cliente que arroja el error
    int socketCliente = socket(AF_INET, SOCK_STREAM, 0);

    if (socketCliente == -1) {
        perror(strcat("Hubo un error al crear el socket del servidor ", nombreCliente));
        exit(-1);
    }

    struct sockaddr_in direccionServer;
    direccionServer.sin_family = AF_INET;
    direccionServer.sin_addr.s_addr = inet_addr(ip);
    direccionServer.sin_port = htons(puerto);
    memset(&(direccionServer.sin_zero), '\0', 8); //se rellena con ceros para que tenga el mismo tamaño que socketaddr

    verificarConnect(socketCliente, &direccionServer);
    // log_info(logger, strcat("Te conectaste con ", nombreCliente));

    return socketCliente;
}

void verificarConnect(int socketCliente, struct sockaddr_in *direccionServer) {
    if (connect(socketCliente, (void*) direccionServer, sizeof((*direccionServer))) == -1) {
        perror("Hubo un problema conectando al servidor: ");
        close(socketCliente);
        exit(-1);
    }
}

int iniciarServidor(char* ip, char* puerto, t_log* logger){
    int socketServidor;
    struct addrinfo hints, *serverInfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &serverInfo);

    // Creamos el socket de escucha del kernel
    socketServidor = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
    // Asociamos el socket a un puerto
    verificarBind(socketServidor, serverInfo);
    // Escuchamos las conexiones entrantes
    verificarListen(socketServidor);

    freeaddrinfo(serverInfo);
    log_trace(logger, "Listo para escuchar a mi cliente!");

    return socketServidor;
}

op_code recibirOperacion(int socketCliente) {
    op_code cod_op;

    if(recv(socketCliente, &cod_op, sizeof(op_code), MSG_WAITALL) > 0) {
     //   printf("recibirOperacion --> cod_op: %d\n", cod_op);
        return cod_op;
    } else {
        close(socketCliente);
        return -1;
    }
}

int esperarCliente(int socketServer, t_log* logger)
{
    // Aceptamos un nuevo cliente
    int socketCliente = accept(socketServer, NULL, NULL);

    if (socketCliente == -1) {
        perror("Hubo un error en aceptar una conexión de la consola: ");
        close(socketServer);
        exit(-1);
    }

    log_info(logger, "Se conecto un cliente!");
    return socketCliente;
}

