#include "../include/funciones.h"

void print_log(int servicio, char* mensaje)
  {
  	char hora[HORA_TAM];
    get_time(hora);

    char serv[SERVICE];
    if(servicio == LOG_USERS)
      sprintf(serv, "users");
    else if(servicio == LOG_GOES)
      sprintf(serv, "goes");
    else
      sprintf(serv, "err");

    char log[TAM];
    sprintf(log, "%s | %s | %s", hora, serv, mensaje);
    printf("%s", log);

    char log_file[LOG];
    sprintf(log_file, "/var/log/%s.log", serv);
    FILE * file = fopen(log_file, "a+");
    if (file == NULL)
      {
        char msg[TAM];
        sprintf(msg, "ERROR %s", log_file);
        perror(msg);
        return;
      }
    fprintf(file, "%s", log);
    fclose(file);
  }

  //función empleada para hacer el timestamp
void get_time(char * tiempo)
  {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(tiempo, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900,
  	 tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  }