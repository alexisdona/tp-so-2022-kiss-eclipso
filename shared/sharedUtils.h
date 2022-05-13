//
// Created by alecho on 8/5/22.
//

#ifndef TP_2022_1C_ECLIPSO_SHAREDUTILS_H
#define TP_2022_1C_ECLIPSO_SHAREDUTILS_H

#include<commons/collections/list.h>

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


#endif //TP_2022_1C_ECLIPSO_SHAREDUTILS_H
