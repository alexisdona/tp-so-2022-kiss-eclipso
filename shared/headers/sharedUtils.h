//
// Created by alecho on 8/5/22.
//

#ifndef TP_2022_1C_ECLIPSO_SHAREDUTILS_H
#define TP_2022_1C_ECLIPSO_SHAREDUTILS_H

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<commons/config.h>
#include <commons/log.h>
#include<stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include<commons/collections/list.h>
#include "protocolo.h"


typedef uint32_t operando;

typedef enum
{
    NO_OP,
    IO,
    READ,
    COPY,
    WRITE,
    EXIT
} instr_code;

typedef struct{
    instr_code codigo_operacion;
    operando parametros[2];
} t_instruccion;

typedef struct
{
    size_t size;
    void* stream;
} t_buffer;

typedef struct
{
    op_code codigo_operacion;
    t_buffer* buffer;
} t_paquete;

typedef struct {
    uint32_t idProceso;
    uint32_t tamanioProceso;
    t_list* listaInstrucciones;
    uint32_t programCounter;
    uint32_t tablaPaginas;
    uint32_t estimacionRafaga;
} t_pcb;

t_config* iniciarConfig(char*);
t_log* iniciarLogger(char*, char*);
t_paquete* crearPaquete(void);
void crearBuffer(t_paquete *);
void* serializarPaquete(t_paquete *, int );
void eliminarPaquete(t_paquete *);
int enviarPaquete(t_paquete*, int);
void enviarMensaje(char*, int);
void recibirMensaje(int, t_log*);
void liberarConexion(int);
void terminarPrograma(uint32_t, t_log*, t_config*);
void* recibirBuffer(size_t, int);
void verificarListen(int);
void verificarBind(int, struct addrinfo*);
int esperar_cliente(int, char*, t_log*);

#endif //TP_2022_1C_ECLIPSO_SHAREDUTILS_H
