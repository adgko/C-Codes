#include "../include/recursos.h"

#define TAM_HORA 20
#define TAM_SERV 40

/*
    Declaración de funciones
*/
struct lista *creanodo(void);
struct lista *insertafinal(struct lista *l, int32_t a,char* b,int32_t c);
struct lista *elimina(struct lista *p, char* a,int32_t b);
void imprimir_log(int productor, char* mensaje,char* ip,int32_t puerto);
void get_time(char*);
void conectar_cliente();