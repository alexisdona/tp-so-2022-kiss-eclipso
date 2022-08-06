# tp-2022-1c-EclipSO

Enunciado:
https://docs.google.com/document/d/17WP76Vsi6ZrYlpYT8xOPXzLf42rQgtyKsOdVkyL5Jj0/edit#heading=h.9vyywhatg0r2

=================================================================================
CONECTARSE A LA VM POR SSH
=================================================================================
Reenvio de puertos
1) Configurar red de la VM con un puerto alto(2222) y GuestPort con 22
ssh utnso@localhost -p 2222

=================================================================================
PREPARAR ENTORNO
=================================================================================
1) Clonar repositorio de las COMMONS:
https://github.com/sisoputnfrba/so-commons-library

2) Instalar COMMONS (meterse en la carpeta del repo):
sudo make install

3) Clonar repositorio TP:
https://github.com/sisoputnfrba/tp-2022-1c-EclipSO

4) Compilar SHARED (meterse en la carpeta del repo)
gcc -c sharedUtils.c -lcommons -lpthread
gcc -shared -o libshared.so sharedUtils.o

5) Copiar libreria en el sistema
sudo cp libshared.so /usr/local/lib/
sudo cp headers/sharedUtils.h /usr/local/include/
sudo ldconfig
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

=================================================================================
COMPILAR MODULO MEMORIA
=================================================================================
gcc -c swap.c -lpthread -lcommons -lshared
gcc -o "memoriaswap" memoria.c -lpthread -lcommons -lshared -lm swap.o

=================================================================================
COMPILAR MODULO CPU
=================================================================================
gcc -o "cpu" cpu.c -lpthread -lcommons -lshared -lm

=================================================================================
COMPILAR MODULO KERNEL
=================================================================================
gcc -c include/utils.c -lpthread -lcommons -lshared
gcc -c planificacion.c -lpthread -lcommons -lshared
gcc -o "kernel" kernel.c -lpthread -lcommons -lshared utilc.o planificacion-o

=================================================================================
COMPILAR MODULO CONSOLA
=================================================================================
gcc -o "consola" consola.c -lpthread -lcommons -lreadline
