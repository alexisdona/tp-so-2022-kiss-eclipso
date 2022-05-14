#include "kernel.h"

int main(void) {
	logger = log_create("kernel.log", "Kernel", 1, LOG_LEVEL_DEBUG);

	int kernel_fd = iniciar_kernel();
	log_info(logger, "Kernel listo para recibir una consola");
	int consola_fd = esperar_consola(kernel_fd);
	int continuar=1;
    t_queue * colaProcesosNew = queue_create();

	while (continuar) { //meter variable para representar por qué va a continuar, hay mas mensajes?
	    printf("entra en el while\n");
		op_code cod_op = recibirOperacion(consola_fd);
		switch (cod_op) {
			case MENSAJE:
                recibirMensaje(consola_fd);
				break;
			case LISTA_INSTRUCCIONES:
			    ;
                t_list* listaInstrucciones = list_create();
                listaInstrucciones = recibirListaInstrucciones(consola_fd);
                int tamanioProceso = recibirTamanioProceso(consola_fd);
                t_pcb* pcb = crearEstructuraPcb(listaInstrucciones, tamanioProceso);
                printf("\nPCB->idProceso:%d", pcb->idProceso);
                printf("\nPCB->tamanioProceso:%d", pcb->tamanioProceso);
                for(uint32_t i=0; i<list_size(listaInstrucciones); i++){
                    t_instruccion *instruccion = list_get(pcb->listaInstrucciones,i);
                    printf("\nEN EL PCB\ninstruccion-->codigoInstruccion->%d\toperando1->%d\toperando2->%d\n",
                           instruccion->codigo_operacion,
                           instruccion->parametros[0],
                           instruccion->parametros[1]);
                }
                printf("PCB->programCounter:%d", pcb->programCounter);
                queue_push(colaProcesosNew, pcb);
                break;
			case -1:
				log_info(logger, "La consola se desconecto.");
				continuar = accion_kernel(consola_fd, kernel_fd);
				break;
			default:
				log_warning(logger,"Operacion desconocida.");
				break;
			}
	}
	return EXIT_SUCCESS;
}

int recibir_opcion() {
	char* opcion = malloc(4);
	log_info(logger, "Ingrese una opcion: ");
	scanf("%s",opcion);
	int op = atoi(opcion);
	free(opcion);
	return op;
}

int validar_y_ejecutar_opcion_consola(int opcion, int consola_fd, int kernel_fd) {

	int continuar=1;

	switch(opcion) {
		case 1:
			log_info(logger,"kernel continua corriendo, esperando nueva consola.");
			consola_fd = esperar_consola(kernel_fd);
			break;
		case 0:
			log_info(logger,"Terminando kernel...");
			close(kernel_fd);
			continuar = 0;
			break;
		default:
			log_error(logger,"Opcion invalida. Volve a intentarlo");
			recibir_opcion();
	}

	return continuar;

}


int accion_kernel(int consola_fd, int kernel_fd) {

	log_info(logger, "¿Desea mantener el kernel corriendo? 1- Si 0- No");
	int opcion = recibir_opcion();

	if (recibirOperacion(consola_fd) == SIN_CONSOLAS) {
	  log_info(logger, "No hay mas clientes conectados.");
	  log_info(logger, "¿Desea mantener el kernel corriendo? 1- Si 0- No");
	  opcion = recibir_opcion();
	  return validar_y_ejecutar_opcion_consola(opcion, consola_fd, kernel_fd);
	}

	return validar_y_ejecutar_opcion_consola(opcion, consola_fd, kernel_fd);

}

void iterator(char* value) {
	log_info(logger,"%s", value);
}

t_pcb* crearEstructuraPcb(t_list* listaInstrucciones, int tamanioProceso) {

    t_pcb *pcb =  malloc(sizeof(t_pcb));
    t_instruccion *instruccion = list_get(listaInstrucciones,0);

    pcb->idProceso = process_get_thread_id();
    pcb->tamanioProceso = tamanioProceso;
    pcb->listaInstrucciones = listaInstrucciones;
    pcb->programCounter= instruccion->codigo_operacion;
    pcb->estimacionRafaga =1; // por ahora dejamos 1 como valor

}
