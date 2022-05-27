
#include <include/planificador.h> 

main() {

}

void planificador() {

  while (1) {
      // consultar planificacion en archivo de config. 
      // flag q consulte si es fifo o no y segun eso correr el proceso q corresponde
  }

}

//mover a utils
void cambiar_estado(t_pcb* p) {
    // cambiar de cola al pcb
}

void correr_proceso_FIFO(t_running_thread* thread_data) { // ready - new y estados suspendidos van a cola fifo
    t_pcb* p = thread_data->p;
    // ver semaforo para q arranque a correr
    while(1) {
       // sacar de la cola de new y poner ready. 
       // hasta que finalice y sacarlo de ready -> pasarlo a la cola de exit 
       // poner en cola de block si es q la CPU lo indica. (E/S)
    }
   
    // destruir semaforo y liberar memoria
}

void correr_proceso_SJF(t_running_thread* thread_data) {
    t_pcb* p = thread_data->p;

    // ver semaforo para q arranque a correr
    // consultar por planificacion bloqueada
    // tenemos que checkear por la estimacion anterior y siguiente 
     // tenemos que ver como mandar a CPU - sockets - interrupt 
    //agregar funcion replanificar_proceso() para mandar a la cola el q corresponda y desaloje al otro proceso
}
