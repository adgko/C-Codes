#include "../include/recursos.h"

int32_t main(){
    printf("%sProductor 1 en linea%s\n",KBLU,KNRM);
    int32_t numero;
    char mensaje[TAM];
    if(mensaje == NULL){
		printf("%sError alocando memoria%s\n",KRED,KNRM);
		exit(1);
	}
    char* primer_parte = "El número aleatorio es ";
    char* salto = "\n";

    /*
        Mientras funcione, genera un número aleatorio y lo guarda como parte del mensaje
        luego lo envía a la cola de mensaje con su ID
    */
    while(1){
        numero = rand();
        sprintf(mensaje,"%s%d%s",primer_parte,numero,salto);
        send_to_queue((long) ID_PROD1, &mensaje[0]);
        sleep(1/X);
    }
}
