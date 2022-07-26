#include "include/swap.h"
#include "include/memoria.h"
#include <fcntl.h>
#include <sys/mman.h>
t_log* logger;
t_config * config;

// Variables globales
t_config* config;
int memoria_fd;
int cliente_fd;
char* ipMemoria, *puertoMemoria;
int marcos_por_proceso, entradas_por_tabla, tamanio_memoria, tamanio_pagina;

t_list* lista_registros_primer_nivel;
t_list* lista_registros_segundo_nivel;
t_list* lista_tablas_primer_nivel;
t_list* lista_tablas_segundo_nivel;
void* espacio_usuario_memoria;
t_bitarray	* frames_disponibles;
void* bloque_frames_lilbres;
char* algoritmo_reemplazo;

// Esta lista serviria solo para un proceso, habria que ver como podemos hacer que sirva con GRADO_MULTIPROGRAMACION procesos
/*
 * Una posibilidad, ya que el GRADO_MULTIPROGRAMACION se settea al iniciar todo el sistema, es inicializar un array de
 * GRADO_MULTIPROGRAMACION tamanio, y en cada posicion, asignar una lista de frames_asignados.
*/
t_list* frames_asignados;

t_list* frames_proceso_0;
t_list* frames_proceso_1;
t_list* frames_proceso_2;
t_list* frames_proceso_3;
t_list* lista_frames_procesos;


void crear_espacio_usuario();

int main(void) {
	//sem_t semMemoria;

	config = iniciarConfig(CONFIG_FILE);
	logger = iniciarLogger("memoria.log", "Memoria");
	ipMemoria= config_get_string_value(config,"IP_MEMORIA");
    puertoMemoria= config_get_string_value(config,"PUERTO_ESCUCHA");
    entradas_por_tabla = config_get_int_value(config,"ENTRADAS_POR_TABLA");
    tamanio_memoria = config_get_int_value(config,"TAM_MEMORIA");
    tamanio_pagina = config_get_int_value(config,"TAM_PAGINA");
    marcos_por_proceso = config_get_int_value(config,"MARCOS_POR_PROCESO");
    algoritmo_reemplazo = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
    preparar_modulo_swap();
    iniciar_estructuras_administrativas_kernel();
    crear_espacio_usuario();
	//sem_init(&semMemoria, 0, 1);

    // Metemos estas inicializaciones dentro de iniciar_estructuras_administrativas_kernel()?
    frames_asignados = list_create();

    frames_proceso_0 = list_create();
    frames_proceso_1 = list_create();
    frames_proceso_2 = list_create();
    frames_proceso_3 = list_create();
    lista_frames_procesos = list_create();
    list_add(lista_frames_procesos, frames_proceso_0);
    list_add(lista_frames_procesos, frames_proceso_1);
    list_add(lista_frames_procesos, frames_proceso_2);
    list_add(lista_frames_procesos, frames_proceso_3);



	// Inicio el servidor
	memoria_fd = iniciarServidor(ipMemoria, puertoMemoria, logger);
	log_info(logger, "Memoria lista para recibir a Kernel o CPU");

    while (1) {
        escuchar_cliente("CPU");
        escuchar_cliente("KERNEL");
    }

	return EXIT_SUCCESS;
}

void procesar_conexion(void* void_args) {

    t_procesar_conexion_attrs* attrs = (t_procesar_conexion_attrs*) void_args;
	t_log* logger = attrs->log;
    int cliente_fd = attrs->fd;
    handshake_cpu_memoria(cliente_fd, tamanio_pagina, entradas_por_tabla, HANDSHAKE_MEMORIA);
    free(attrs);

      while(cliente_fd != -1) {
	        op_code cod_op = recibirOperacion(cliente_fd);
            switch (cod_op) {
                case MENSAJE:
                    recibirMensaje(cliente_fd, logger);
                    break;
                case ESCRIBIR_MEMORIA:
                    ;
                    void* buffer_escritura = recibirBuffer(cliente_fd);
                    uint32_t marco_escritura;
                    uint32_t desplazamiento_escritura;
                    uint32_t valor_a_escribir;
                    memcpy(&marco_escritura, buffer_escritura, sizeof(uint32_t));
                    memcpy(&desplazamiento_escritura, (buffer_escritura+sizeof(uint32_t)), sizeof(uint32_t));
                    memcpy(&valor_a_escribir, (buffer_escritura+sizeof(uint32_t)+sizeof(uint32_t)), sizeof(uint32_t));
                    //escribo en el espacio de usuario el valor
                    uint32_t desplazamiento_final_escritura = (marco_escritura*tamanio_pagina+desplazamiento_escritura);
                    memcpy((espacio_usuario_memoria+desplazamiento_final_escritura), &valor_a_escribir, sizeof(uint32_t))      ;
                    enviarMensaje("Ya escribí en memoria!", cliente_fd);
                    break;
                case LEER_MEMORIA:
                    ;
                    void* buffer_lectura = recibirBuffer(cliente_fd);
                    uint32_t marco_lectura;
                    uint32_t desplazamiento;
                    memcpy(&marco_lectura, buffer_lectura, sizeof(uint32_t));
                    memcpy(&desplazamiento, (buffer_lectura+sizeof(uint32_t)), sizeof(uint32_t));
                    uint32_t desplazamiento_final_lectura = (marco_lectura*tamanio_pagina+desplazamiento);
                    uint32_t* valor = (uint32_t*) (espacio_usuario_memoria + desplazamiento_final_lectura);
                    enviar_entero(cliente_fd, *valor, LEER_MEMORIA);
                    break;
                case SWAPEAR_PROCESO:
                	log_info(logger, "Recibi un PCB a swapear");
                	t_pcb* pcb = recibirPCB(cliente_fd);
                	swapear_proceso(pcb);
                	break;
                case CREAR_ESTRUCTURAS_ADMIN:
                    ;
                    t_pcb* pcb_kernel = recibirPCB(cliente_fd);
                    crear_archivo_swap(pcb_kernel->idProceso, pcb_kernel->tamanioProceso);
                    pcb_kernel->tablaPaginas = crear_estructuras_administrativas_proceso(pcb_kernel->tamanioProceso) - 1;
                    enviarPCB(cliente_fd,pcb_kernel, ACTUALIZAR_INDICE_TABLA_PAGINAS);
                    break;
                case OBTENER_ENTRADA_SEGUNDO_NIVEL:
                    ;
                    void* buffer_tabla_segundo_nivel = recibirBuffer(cliente_fd);

                    size_t nro_tabla_primer_nivel;
                    uint32_t entrada_tabla_primer_nivel;
                    memcpy(&nro_tabla_primer_nivel, buffer_tabla_segundo_nivel, sizeof(size_t));
                    memcpy(&entrada_tabla_primer_nivel, buffer_tabla_segundo_nivel + sizeof(size_t), sizeof(uint32_t));
                    t_registro_primer_nivel* registro_primer_nivel = (list_get(list_get(lista_tablas_primer_nivel, nro_tabla_primer_nivel), entrada_tabla_primer_nivel)) ;
                    uint32_t nro_tabla_segundo_nivel = registro_primer_nivel->nro_tabla_segundo_nivel;
                    enviar_entero(cliente_fd, nro_tabla_segundo_nivel, OBTENER_ENTRADA_SEGUNDO_NIVEL);
                    break;
                case OBTENER_MARCO:
                    ;
                    void* buffer_marco = recibirBuffer(cliente_fd);
                    size_t id_proceso;
                    uint32_t entrada_tabla_segundo_nivel;
                    uint32_t numero_pagina;
                    uint32_t nro_tabla_segundo_nivel_obtener_marco;
                    int despl = 0;
                    uint32_t marco;
                    memcpy(&id_proceso, buffer_marco+despl, sizeof(size_t));
                    despl+= sizeof(size_t);
                    memcpy(&nro_tabla_segundo_nivel_obtener_marco, buffer_marco+despl, sizeof(uint32_t));
                    despl+= sizeof(uint32_t);
                    memcpy(&entrada_tabla_segundo_nivel, buffer_marco+despl, sizeof(uint32_t));
                    despl+= sizeof(uint32_t);
                    memcpy(&numero_pagina, buffer_marco+despl, sizeof(uint32_t));

                    t_registro_segundo_nivel* registro_segundo_nivel = list_get(list_get(lista_tablas_segundo_nivel, nro_tabla_segundo_nivel_obtener_marco), entrada_tabla_segundo_nivel);

                    if(registro_segundo_nivel->presencia) {
                        marco = registro_segundo_nivel->frame;
                    }
                    else {
                        void *bloque = obtener_bloque_proceso_desde_swap(id_proceso, numero_pagina);

                        // primero validar que el proceso tenga marcos disponibles
                        uint32_t cantidad_marcos_ocupados_proceso = obtener_cantidad_marcos_ocupados(entrada_tabla_primer_nivel);
                        printf("\nCantidad de marcos ocupados: %d\n", cantidad_marcos_ocupados_proceso);
                        if (cantidad_marcos_ocupados_proceso < marcos_por_proceso) {

                            uint32_t nro_frame_libre = obtener_numero_frame_libre();

                            registro_segundo_nivel->presencia = true;
                            registro_segundo_nivel->frame = nro_frame_libre;
                            marco = registro_segundo_nivel->frame;
                            memcpy(espacio_usuario_memoria + marco * tamanio_pagina, bloque, tamanio_pagina); //agrego bloque en el espacio de usuario

                        } else {
                            // algoritmo de reemplazo
                        }
                    }

                    enviar_entero(cliente_fd, marco, OBTENER_MARCO);
                    break;
                case -1:
                    log_info(logger, "El cliente se desconectó");
                    cliente_fd = -1;
                    break;
                default:
                    log_warning(logger, "Operacion desconocida.");
                    break;
            }

	}
}

int escuchar_cliente(char *nombre_cliente) {
    int cliente = esperarCliente(memoria_fd, logger);
    if (cliente != -1) {
        pthread_t hilo;
        t_procesar_conexion_attrs* attrs = malloc(sizeof(t_procesar_conexion_attrs));
        attrs->log = logger;
        attrs->fd = cliente;
        pthread_create(&hilo, NULL, (void*) procesar_conexion, (void*) attrs);
        pthread_detach(hilo);
        return 1;
    }
    return 0;
}

void preparar_modulo_swap(){
	cola_swap = queue_create();
	PATH_SWAP = config_get_string_value(config,"PATH_SWAP");
	string_append(&PATH_SWAP,"/");
	RETARDO_SWAP = config_get_int_value(config,"RETARDO_SWAP");
}

size_t crear_estructuras_administrativas_proceso(size_t tamanio_proceso) {

    int cantidad_entradas_tabla_segundo_nivel = MAX((tamanio_proceso/tamanio_pagina),1);
    int indice_primer_nivel = 0;
    int indice_segundo_nivel=0;

    t_registro_segundo_nivel* registro_tabla_segundo_nivel;
    t_registro_primer_nivel* registro_tabla_primer_nivel;

    while(indice_segundo_nivel < cantidad_entradas_tabla_segundo_nivel) {
        registro_tabla_primer_nivel = malloc(sizeof(t_registro_primer_nivel));
        registro_tabla_primer_nivel->indice = indice_primer_nivel;
        registro_tabla_primer_nivel->nro_tabla_segundo_nivel = list_size(lista_tablas_segundo_nivel);
        list_add(lista_registros_primer_nivel, registro_tabla_primer_nivel);

        for (int j=0; j<entradas_por_tabla; j++){
            registro_tabla_segundo_nivel = malloc(sizeof(t_registro_segundo_nivel));
            registro_tabla_segundo_nivel->indice = j;
            registro_tabla_segundo_nivel->frame = 0;
            registro_tabla_segundo_nivel->modificado=false;
            registro_tabla_segundo_nivel->uso=false;
            registro_tabla_segundo_nivel->presencia=false;

            list_add(lista_registros_segundo_nivel, registro_tabla_segundo_nivel);
            indice_segundo_nivel++;
        }
        indice_primer_nivel++;
        list_add(lista_tablas_segundo_nivel, list_duplicate(lista_registros_segundo_nivel));
        list_clean(lista_registros_segundo_nivel);
    }
    list_add(lista_tablas_primer_nivel, lista_registros_primer_nivel);
    return list_size(lista_tablas_primer_nivel);
}

void crear_bitmap_frames_libres() {
    uint32_t tamanio_bit_array = tamanio_memoria / tamanio_pagina;
    bloque_frames_lilbres = malloc(tamanio_bit_array);
    frames_disponibles = bitarray_create_with_mode(bloque_frames_lilbres, tamanio_bit_array, LSB_FIRST);
}

void iniciar_estructuras_administrativas_kernel() {
    lista_registros_primer_nivel = list_create();
    lista_registros_segundo_nivel = list_create();
    lista_tablas_primer_nivel = list_create();
    lista_tablas_segundo_nivel = list_create();
    crear_bitmap_frames_libres();

}

void crear_espacio_usuario() {
    espacio_usuario_memoria = malloc(tamanio_memoria);
    uint32_t valor=0;
    for(int i=0; i< tamanio_memoria/sizeof(uint32_t); i++) {
        valor = i;
        memcpy(espacio_usuario_memoria + sizeof(uint32_t) *i , &valor, sizeof(uint32_t));
    }
  /*  printf("\nMEMORIA --> IMPRIMO VALORES EN ESPACIO DE USUARIO\n");
    for(int i=0; i< tamanio_memoria/sizeof(uint32_t); i++) {
        uint32_t* apuntado=  espacio_usuario_memoria + i*sizeof(uint32_t);
       printf("\nvalor apuntado en posición %d-->%d",i, *apuntado);
    }
    */
}

void* obtener_bloque_proceso_desde_swap(size_t id_proceso, uint32_t numero_pagina) {

    char *ruta = string_new();
    string_append(&ruta, PATH_SWAP);
    verificar_carpeta_swap(ruta);

    string_append(&ruta, string_itoa(id_proceso));
    string_append(&ruta, ".swap");
     int ubicacion_bloque = numero_pagina*tamanio_pagina;
     int archivo_swap = open(ruta, O_RDONLY, S_IRWXU);
     struct stat sb;
     if (fstat(archivo_swap,&sb) == -1) {
         perror("No se pudo obtener el size del archivo swap: ");
     }
     void* contenido_swap = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, archivo_swap, 0);
     void* bloque = malloc(tamanio_pagina);
     memcpy(bloque, contenido_swap+ubicacion_bloque, tamanio_pagina);
     return bloque; //devuelve la pagina entera que es del tamano de pagina
}

uint32_t obtener_numero_frame_libre() {

    for(int i= 0; frames_disponibles->size; i++) {
        if ( bitarray_test_bit(frames_disponibles, i) == 0) {
            bitarray_set_bit(frames_disponibles, i);
            return i;
        }
    }
    return -1;
}

uint32_t obtener_cantidad_marcos_ocupados(size_t nro_tabla_primer_nivel) {
    uint32_t cantidad_marcos_ocupados = 0;
    t_list* tabla_primer_nivel = list_get(lista_tablas_primer_nivel, nro_tabla_primer_nivel);

    for(int i=0; i<tabla_primer_nivel->elements_count; i++){
        t_registro_primer_nivel* registro_primer_nivel = list_get(tabla_primer_nivel,i);
        t_list* lista_registros_segundo_nivel = list_get(lista_tablas_segundo_nivel, registro_primer_nivel->nro_tabla_segundo_nivel);

        for(int j=0; j<lista_registros_segundo_nivel->elements_count; j++){
            t_registro_segundo_nivel* registro_segundo_nivel = list_get(lista_registros_segundo_nivel,j);
            if(registro_segundo_nivel->presencia == 1) {
                cantidad_marcos_ocupados+=1;
            }
        }
    }
    return cantidad_marcos_ocupados;
}


//********************************* ALGORITMOS DE SUSTITUCION DE PAGINAS ***********************************

void sustitucion_paginas(uint32_t tabla_primer_nivel, uint32_t numero_tabla_segundo_nivel, uint32_t registro_segundo_nivel) {
	// Busqueda de primer frame libre en bitarray de frames_disponibles
	uint32_t frame_libre;

	if (strcmp("CLOCK", algoritmo_reemplazo)) {
		algoritmo_clock(tabla_primer_nivel, numero_tabla_segundo_nivel, registro_segundo_nivel, frame_libre);
	}
	else if (strcmp("CLOCK-M", algoritmo_reemplazo)) {
		algoritmo_clock_modificado(tabla_primer_nivel, numero_tabla_segundo_nivel, registro_segundo_nivel);
	}
}

void algoritmo_clock(uint32_t indice_tabla_primer_nivel, uint32_t indice_primer_nivel, uint32_t indice_segundo_nivel, uint32_t frame_asignado) {
	t_list* tabla_primer_nivel = list_get(lista_tablas_primer_nivel, indice_tabla_primer_nivel);
	t_registro_primer_nivel* registro_primer_nivel = list_get(tabla_primer_nivel, indice_primer_nivel);
	t_list* tabla_segundo_nivel = list_get(lista_tablas_segundo_nivel, registro_primer_nivel->nro_tabla_segundo_nivel);
	t_registro_segundo_nivel* registro_segundo_nivel = list_get(tabla_segundo_nivel, indice_segundo_nivel);

	// Variables auxiliares
	t_registro_segundo_nivel* registro_segundo_nivel_victima;

	// Uso marcos por proceso o list_size(frames_asignados)?
	// Yo se que siempre voy a tener que reemplazar cuando supere el maximo de marcos por proceso
	for (int i=0; i < marcos_por_proceso; i++) {

		// Siempre tomamos el elemento 0 porque consideramos el list->head como el puntero
		t_frame_clock* frame = list_get(frames_asignados, 0);

		if (es_victima_clock(frame)) {
			/*
			 * En base al numero de pagina, calcula el indice de la tabla
			 * de paginas de primer nivel y de segundo nivel para actualizar sus bits (U/M/P)
			 *
			 * o que directamente devuelva el puntero al registro victima de la tabla de segundo nivel
			 */
			registro_segundo_nivel_victima = obtener_registro_segundo_nivel(indice_tabla_primer_nivel, frame->numero_pagina);

			// Limpieza de registro victima
			// frames_disponibles ----> registro_segundo_nivel_victima->frame = 0;
			registro_segundo_nivel_victima->presencia = 0;
			registro_segundo_nivel_victima->uso = 0;
			if (registro_segundo_nivel_victima->modificado == 1) {
				// Actualizar pagina en swap
				registro_segundo_nivel_victima->modificado = 0;
			}

			// Carga de pagina solicitada
			// frames_disponibles ----> registro_segundo_nivel->frame = 1;
			registro_segundo_nivel->frame = frame->numero_frame;	// Le asigno el frame que fue desocupado y elegido como victima
			registro_segundo_nivel->uso = 1;
			registro_segundo_nivel->presencia = 1;
			// El bit de modificado se tiene que settear al momento de leer el codigo de operacion
			// Si es WRITE => m = 1

			frame->numero_pagina =
			frame->uso = 1;
			list_add(frames_asignados, frame);
			list_remove(frames_asignados, 0);

			break;
		} else {
			frame->uso=0;
			list_add(frames_asignados, frame);
			// Confirmar que el remove reposiciona el list->head
			// Hacemos remove_and_destroy o algo de eso?
			list_remove(frames_asignados, 0);
		}
	}

}

void algoritmo_clock_modificado(uint32_t tabla_primer_nivel, uint32_t numero_tabla_segundo_nivel, uint32_t registro_segundo_nivel) {

}

int es_victima_clock(t_frame_clock* frame) {
	return frame->presencia == 1 && frame->uso == 0;
}

int es_victima_clock_modificado_um(t_registro_segundo_nivel* registro) {
	return registro->presencia == 1 && registro->uso == 0 && registro->modificado == 0;
}

int es_victima_clock_modificado_u(t_registro_segundo_nivel* registro) {
	return registro->presencia == 1 && registro->uso == 0 && registro->modificado == 1;
}

void cargar_pagina(uint32_t frame_asignado) {

}
