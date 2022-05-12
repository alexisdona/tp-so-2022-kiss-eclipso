//
// Created by alecho on 8/5/22.
//

#ifndef TP_2022_1C_ECLIPSO_SHAREDUTILS_H
#define TP_2022_1C_ECLIPSO_SHAREDUTILS_H
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

#endif //TP_2022_1C_ECLIPSO_SHAREDUTILS_H
