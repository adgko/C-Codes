#include <stdio.h>
#include <stdlib.h>
#include <ulfius.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>     //el directorio actual
#include "../include/funciones.h"

#define PUERTO 9014
#define BUFFER_SIZE 1024
#define TAM 1024
#define BYTES_TO_MB 1048576
#define PATH_FILES "/var/archivos"
#define LOG_GOES 2


typedef struct {
  int32_t index;		    //el indice sirve para buscar y ordenar los archivos
  char nombre[TAM];			//nombre del archivo
  float size;				//tamaño del archivo
  char link[TAM];
} Archivo;

json_t *files;          // el campo donde voy guardando los archivos es global
                        // para que pueda invocarlo en main y luego llamarlo de nuevo frente a actualizaciones

int32_t inicializar_server(struct _u_instance*);
void finalizar_server(struct _u_instance*);
int callback_goes_get(const struct _u_request*, struct _u_response*, void*);
int32_t get_files_list(json_t*);
void calc_size(int32_t,char*);
long gregorian_calendar_to_jd(int y, int m, int d);
int32_t callback_goes_post ( const struct _u_request *, struct _u_response *, void *);

int32_t main(void){
    struct _u_instance instance;
    int32_t fin = 0;

    //inicializa la instancia de ulfius 
    if(inicializar_server(&instance) == 1){
        char mensaje[BUFFER_SIZE];
        sprintf(mensaje, "Error iniciando ulfius\n");
        printf("%s",mensaje);
        print_log(LOG_GOES, mensaje);
        exit(1);
    }
    files = json_array();     
    if( get_files_list(files) == 1)
      {
        return 1;
      }    
    //se queda prendido esperando peticiones
    while(fin == 0) sleep(200);
    //cierra la instancia de ulfius
    finalizar_server(&instance);

    return 0;
}

//incializa ufius asignandole el puerto y definiendo las llamadas que voy a usar
int32_t inicializar_server(struct _u_instance * instance){
    if(ulfius_init_instance(instance,PUERTO,NULL,NULL) != U_OK){
        return 1;
    }

    ulfius_add_endpoint_by_val( instance, "GET", "/get_goes",
                                NULL, 0, &callback_goes_get, NULL);
    ulfius_add_endpoint_by_val( instance, "POST", "/get_goes",
                                NULL, 0, &callback_goes_post, NULL);

    if (ulfius_start_framework(instance) == U_OK)
      {
        char mensaje[BUFFER_SIZE];
        sprintf(mensaje, "Servicio GOES escuchando en puerto %d\n", instance->port);
        printf("%s",mensaje);
        print_log(LOG_GOES, mensaje);
        return 0;
      }
    else
      {
        char mensaje[BUFFER_SIZE];
        sprintf(mensaje, "ERROR: iniciando framework uflius\n");
        printf("%s",mensaje);
        print_log(LOG_GOES, mensaje);
        return 1;
      }
}

//cierra y limpia el espacio de memoria de la instancia
void finalizar_server(struct _u_instance * instance)
  {
    char mensaje[BUFFER_SIZE];
    sprintf(mensaje, "Servidor cerrándose\n");
    printf("%s",mensaje);
    print_log(LOG_GOES, mensaje);

    ulfius_stop_framework(instance);
    ulfius_clean_instance(instance);
  }

 #pragma GCC diagnostic ignored "-Wunused-parameter"
int32_t callback_goes_get ( const struct _u_request * request,
                struct _u_response * response, void * user_data){
    #pragma GCC diagnostic pop
    json_t *respuesta = json_pack("{s:o}", "data", files);
    ulfius_set_json_body_response( response, 200, respuesta );
    return 0;            
}
//recorre el directorio donde están los archivos y se van almacenando sus datos,
// indice, nombre y tamaño, para ser enviado como respuesta
// se hardcodeó la cantidad de archivos
int32_t get_files_list(json_t *body){
    DIR *d;
    struct dirent *dir;
    int i = 0;
    d = opendir(PATH_FILES);
    if(d == NULL){
        char mensaje[BUFFER_SIZE];
        sprintf(mensaje, "No se pudo abrir el directorio\n");
        printf("%s",mensaje);
        print_log(LOG_GOES, mensaje);
        exit(1);
    }
      
      while((dir = readdir(d))){
        if((strcmp(dir->d_name,".") != 0) && (strcmp(dir->d_name,"..") != 0)){
          //char aux[TAM];
          //char* direccion = "localhost/api/data/";
          //sprintf(aux,"%s%s",direccion,dir->d_name);
          json_t *files = json_pack("{s:i,s:s}","id",i,"archivo",dir->d_name);
          json_array_append(body,files);
          i++;
        }
      }

        char mensaje[BUFFER_SIZE];
        sprintf(mensaje, "archivos leidos y almacenados\n");
        printf("%s",mensaje);
        print_log(LOG_GOES, mensaje);
      closedir(d);
      return 0;
}


//Recibe la petición con el producto y la fecha
// si está, devuelve el link
// si no está, avisa que no está y que lo pidan luego. Mientras, lo descarga
#pragma GCC diagnostic ignored "-Wunused-parameter"
int32_t callback_goes_post ( const struct _u_request * request,
                struct _u_response * response, void * user_data){

    #pragma GCC diagnostic pop
    json_error_t * json_error = malloc( sizeof (struct json_error_t));
    json_t * json_request = ulfius_get_json_body_request(request, json_error);

    //si está incompleta la petición, avisa
    if( request == NULL)
      {
        char mensaje[BUFFER_SIZE];
        sprintf(mensaje, "ERROR: %s\n", json_error->text);
        printf("%s",mensaje);
        print_log(LOG_GOES, mensaje);  
        free(json_error);
        return 1;
      }
    //recibe producto que quiere el user y la fecha que quiera
    json_t * product = json_object();
    product = json_object_get(json_request, "product");
    json_t * datetime = json_object();
    datetime = json_object_get(json_request, "datetime");

//prepara el link de descarga, armado con el link que pasó Agus
    char aux[1024*2];
    char goes[1024] = "https://noaa-goes16.s3.amazonaws.com";
    char producto[100];
    sprintf(producto, "%s", json_string_value(product) );
    char date[20];
    sprintf(date, "%s", json_string_value(datetime) );

    //char anio[10] = "2021";
    char *anio = strtok(date,"%");
    //char juliano[10] = "003";
    char *mes = strtok(NULL,"%");
    //char hor[10] = "00";
    char *dia = strtok(NULL,"%");
    char *hor = strtok(NULL,"%");
    char *otradia = malloc(sizeof("0")+sizeof(hor));
    //con la fecha que le llega, hace el día juliano para laconsulta
    //long juliano = gregorian_calendar_to_jd(atoi(anio), atoi(mes), atoi(dia));
    // tengo que agregarle un 0 porque así lo pide aws
    long juliano = (atoi(mes)-1)*30+atoi(dia);
    char *juli = malloc(sizeof("0")+sizeof(juliano));
    sprintf(juli,"%ld",juliano);
    if(juliano < 100){sprintf(juli,"0%ld",juliano);}
    if(juliano < 10){sprintf(juli,"0%ld",juliano);}
    //if( atoi(dia) < 10){sprintf(otradia,"0%s",dia);}

    // aca pregunta si está 
    json_t *respuesta = json_array();
    json_t *respuesta_encontrado = json_array();
    json_t *mensaje_aux = json_pack("{s:s}","mensaje","elarchivo que busca no está, por lo que se descargará. Pruebe de nuevo más tarde.");
    json_array_append(respuesta,mensaje_aux);
    char directory[1024];
    getcwd(directory,sizeof(directory));
    DIR *d = opendir(directory);
    int32_t encontrado = 0;
    if(d == NULL){
        char mensaje[BUFFER_SIZE];
        sprintf(mensaje, "No se pudo abrir el directorio\n");
        printf("%s",mensaje);
        print_log(LOG_GOES, mensaje);
        exit(1);
    }
    struct dirent *dir;
    char aux2[1024];
    char aux3[1024];
    char aux4[1024*2];
    char nombre_producto[1024];
    char producto_aux[1024];
    char fecha_producto[1024];

//recorre el directorio y compara el nombre de este con el producto y fecha
    while((dir = readdir(d)) != NULL){
      if((strcmp(dir->d_name,".") != 0) && (strcmp(dir->d_name,"..") != 0)){
        char *aux_str;
        char nombre[1024];
        sprintf(nombre,"%s",dir->d_name);
        aux_str = strtok(dir->d_name,"_");
        aux_str = strtok(NULL,"_");
        sprintf(nombre_producto,"%s",aux_str);  //saco el producto
        aux_str = strtok(NULL,"_");
        aux_str = strtok(NULL,"_");
        sprintf(aux3,"%s",aux_str);   //saco la fecha
        printf("1:%s\n ",aux3);
        sprintf(aux2,"s%s%s%s",anio,juli,hor);  //armo la fecha que me piden
        printf("2:%s\n ",aux2);

        sprintf(producto_aux,"%s-M6",producto);
        sprintf(fecha_producto,"%s",aux2);
        //comparo a ver si es el mismo producto
        if(strcmp(nombre_producto,producto_aux) == 0){
          //comparo a ver si es la misma fecha con strstr, que es buscar una subcadena dentro de la cadena
          if(strstr(aux3,aux2) != NULL){
          sprintf(aux4,"%s%s","http://localhost/api/data/",nombre);   //armo link
              json_t *link = json_pack("{s:s}","link",aux4);          // lo guardo en un json pack y le hago append
              json_array_append(respuesta_encontrado,link);
              encontrado = 1;                                         //aviso que lo encontré y salgo del while
              break;
              }
        }

      }
    }
    //si lo encuentro, aviso y mando el link que armé 
    if(encontrado == 1){
      char mensaje[BUFFER_SIZE];
      sprintf(mensaje, "Se encontró el archivo \n");
      printf("%s",mensaje);
      print_log(LOG_GOES, mensaje);
      ulfius_set_json_body_response( response, 200, respuesta_encontrado );
      
    }//sino, lo pido para descargar
    else{

      ulfius_set_json_body_response( response, 200, respuesta );  //envío el mensaje para que sepa que será descargado
      //crea un proceso hijo para poder descargar en segundo plano
      int32_t pid;
      pid = fork();
      if(pid == 0){

        
        //Pide el listado y con los datos que llegaron y lo guarda en un archivo
        sprintf(aux,"aws s3 ls noaa-goes16/%s/%s/%s/%s/ --human-readable --no-sign-request >> archivos",producto,anio,juli,hor);
        printf("%s\n",aux);
        system(aux);
        //abre el archivo, toma la primera fila y obtiene el nombre del archivo
        FILE *p;
        char *fecha,*hora,*tamanio,*formato,*nombre;
        char var[1024];
        p = fopen("archivos","r");
        fgets(var, sizeof(var), p);
        fecha = strtok(var," ");
        hora = strtok(NULL, " ");
        tamanio = strtok(NULL, " ");
        formato = strtok(NULL, " ");
        nombre = strtok(NULL, " ");
        //no pude hace otra forma de sacar esto del warning
        if(fecha == NULL){}
        if(hora == NULL){}
        if(tamanio == NULL){}
        if(formato == NULL){}

        //arma el link con el nombre y los datos y lo baja ocn wget
        sprintf(aux,"wget %s/%s/%s/%s/%s/%s",goes,producto,anio,juli,hor,nombre );
        printf("%s\n",aux);
        system(aux);
        //cierra el archivo y lo borra
        fclose(p);
        system("rm archivos");
        //sprintf(aux,"cp %s /var/archivos/",nombre);
        //system(aux);
       // sprintf(aux,"rm %s ",nombre);
       // system(aux);

        char mensaje[BUFFER_SIZE];
        sprintf(mensaje,"Se descargó el archivo %s \n",nombre);
        printf("%s",mensaje);
        print_log(LOG_GOES, mensaje);

        if( get_files_list(files) == 1)
        {
          return 1;
        }   
      }
    }
    




    return 0;
}


//ingresa año, mes y día en gregoriano, y lo pasa a Juliano
// el offset desumar 8000 al año, es porque el calendario gregoriano toma hasta el 4714 aC
// luego se le resta 1200820 al calculo para compensar
long gregorian_calendar_to_jd(int y, int m, int d)
    {
	y+=8000;
	if(m<3) { y--; m+=12; }
	return (y*365) +(y/4) -(y/100) +(y/400) -1200820
              +(m*153+3)/5-92
              +d-1
	;
    }
