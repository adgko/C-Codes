#include "recursos.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ioctl.h> //para poner no bloqueante
#include <sys/poll.h> 
#include <arpa/inet.h>
#include <signal.h>
#include <sys/sendfile.h>
#include <sys/stat.h>   //para open()
#include <fcntl.h>      // para open()


/*
    Paths de los binarios productores y CLI
*/
#define PROD1_PATH "prod1"
#define PROD2_PATH "prod2"
#define PROD3_PATH "prod3"
#define FILE_PATH "file"
#define CLI_PATH    "cli"
#define TAM_MAX_MENSAJE 10240
/*
    Máxima cantidad de clientes soportada
*/
#define MAX_CLIENTES 5000
/*
    Constantes para determinar tiempo antes de cerrar la conexión
*/
#define MINUTES     3
#define SECONDS     60
#define MILISECONDS 1000

void validar_comando(char *a, char *b, char *c);

void configurar_socket();
void escuchando();
void read_log();
void conectar_cliente();
void enviar_log();