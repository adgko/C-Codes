#include "../include/recursos.h"

#define SERVER_OK 	  0
#define SERVER_ER     1
#define linea_cores   12

int32_t main(){
    printf("%sProductor 3 en linea%s\n",KMAG,KNRM);
    char buffer[TAM];
    char cores[TAM];
    if(buffer == NULL){
        printf("%sError alocando memoria%s\n",KRED,KNRM);
		    exit(1);
	}
    while(1){
        double load_avg[1];
        getloadavg(load_avg, 1);

        char * path = "/proc/cpuinfo";
        FILE* file = fopen(path, "r");
        if( file == NULL)
        {
            perror("no se pudo abrir el archivo\n");
            return SERVER_ER;
        }

        char linea[TAM];
        for( int i = 0; i <= linea_cores; i++)
        fgets(linea, TAM, file);

        char * aux = strtok(linea, ":");
        aux = strtok(NULL, ":");
        for( size_t i = 1; i < strlen(aux); i++ )
        {
            if( aux[i] == '\n' )
            {
                cores[i - 1] = '\0';
                break;
            }
            else
            cores[i - 1] = aux[i];
        }
        fclose(file);

        load_avg[0] = load_avg[0]/cores[0];

        sprintf(buffer, "La carga es %f\n", load_avg[0]);
        //printf("%s",buffer);
        send_to_queue((long) ID_PROD3, &buffer[0]);
        sleep(1/Z);
    }

    return SERVER_OK;
}
