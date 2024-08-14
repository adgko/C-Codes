
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <zip.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#define TAM 1024

/*
    Valores empleados para el calculo de tasa de envío de mensajes
    en los productores
*/
#define X 0.5
#define Y 0.2
#define Z 0.1

/*
    etiquetas de los productores para la cola de mensajes
*/
#define ID_PROD1    1
#define ID_PROD2    2
#define ID_PROD3    3
#define ID_CLI      4
#define ID_CLI_RES 5

/*
    Etiquetas para los logs
*/
#define LOG_PROD_1 1
#define LOG_PROD_2 2
#define LOG_PROD_3 3
#define LOG_CLI    4

/*
    Etiquetas para los colores de los mensajes
*/
#define GREAT   1
#define OK      2
#define WARNING 3
#define ERROR   4

/*
    Tags para cerrar las conexiones
*/
#define TRUE    1
#define FALSE   0

/*
	Variables empleadas para imprimir en colores
*/
#define KNRM  "\x1B[0m"	    //normal
#define KRED  "\x1B[31m"	//rojo
#define KGRN  "\x1B[32m"	//verde
#define KYEL  "\x1B[33m"	//amarillo
#define KBLU  "\x1B[34m"	//azul
#define KMAG  "\x1B[35m"	//magenta
#define KCYN  "\x1B[36m"	//cyan
#define KWHT  "\x1B[37m"	//blanco

#define LOG_PATH "../archivos/logs"

#define LOG_PATH_1 "../archivos/logs/productor_1.log"
#define LOG_PATH_2 "../archivos/logs/productor_2.log"
#define LOG_PATH_3 "../archivos/logs/productor_3.log"
#define LOG_PATH_log "../archivos/logs/cli.log"

/*
    Si no se declaran acá no las reconoce
*/
int32_t get_queue();
int32_t send_to_queue(long, char [TAM] );
char* recive_from_queue(long , int32_t );
void imprimir_log(int productor, char* mensaje,char* ip,int32_t puerto);
void zipear();
void vaciar_cola();
/*
    Estructura de los nodos de la lista
*/
struct lista { /* lista simple enlazada */
  int32_t fd;
  char* ip;
  int32_t port;
  int subs_1;
  int subs_2;
  int subs_3;
  int desconectado;
  struct lista *sig;
};

/*
    funciones de lista
*/
int longitudl(struct lista *l);
struct lista *creanodo(void);
struct lista *insertafinal(struct lista *l, int32_t a,char* b,int32_t c);
struct lista *elimina(struct lista *p, char* a,int32_t b);
void ImprimirElementosLista (struct lista *a);



/*
    MD5
*/

char *md5(const char*, int);