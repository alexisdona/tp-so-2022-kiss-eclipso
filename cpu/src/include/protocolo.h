#ifndef SRC_INCLUDE_PROTOCOLO_H_
#define SRC_INCLUDE_PROTOCOLO_H_

typedef enum
{
    MENSAJE,
    PAQUETE
} op_code;

typedef struct
{

} estructura_kernel;

typedef struct
{
	uint32_t direcciones_paginacion;
} estructura_memoria;

#endif /* SRC_INCLUDE_PROTOCOLO_H_ */
