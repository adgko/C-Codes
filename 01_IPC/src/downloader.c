#include "../include/cliente.h"
#define puerto_downloader 8021
#define puerto_files 8020
#define archivo_destino  "/your/directory"

int32_t sockfd, sockfil;
struct sockaddr_in serv_addr;

uint32_t client_len; // tamaño de la dirección del cliente
char buffer[TAM];
ssize_t n;     // hubo que declarar n como ssize_t para que no pierda información al usarse en send() y recv()
long *logsize; // para guardar size de log
char *logmd5;  // para guardar md5 de log
//int size;
size_t size;
int32_t main()
{

    configurar_socket(); // configura y abre el socket para recibir

    conect_to_files();

    // En el cliente (downloader)
    printf("Intentando conectar al servidor en el puerto %d\n", puerto_files);

    memset(buffer, 0, TAM);

    n = recv(sockfil, buffer, TAM - 1, 0);
    if (n < 0)
    {
        perror("lectura de socket");
        exit(1);
    }

    printf("%sRecibí: %s%s\n", KBLU, buffer, KNRM);

    memset(buffer, 0, TAM);

    // recibe tamaño
    n = recv(sockfil, buffer, TAM - 1, 0);
    if (n < 0)
    {
        perror("lectura de socket");
        exit(1);
    }

    //printf("%sRecibí: %s%s\n", KBLU, buffer, KNRM);

    char tamanio[TAM];
     sprintf(tamanio,"%s",buffer);
     printf("%sRecibí: %s%s\n", KBLU, tamanio, KNRM);

    //size = atoi(tamanio);
    size = strtoul(tamanio, NULL, 10);




    // envía confirmaciòn
    n = send(sockfil, "mandame el hash", strlen("mandame el hash"), 0);
    if (n < 0)
    {
        perror("fallo en enviar info");
    }

    memset(buffer, 0, TAM);

    //recibe hash md5
    n = recv(sockfil, buffer, TAM - 1, 0);
    if (n < 0)
    {
        perror("lectura de socket");
        exit(1);
    }

    //printf("%sRecibí: %s%s\n", KBLU, buffer, KNRM);

     char md5_recv[TAM];
     sprintf(md5_recv,"%s",buffer);
     printf("%sRecibí: %s%s\n", KBLU, md5_recv, KNRM);

     //envía confirmación para que le envíen archivo
     n = send(sockfil, "mandame el archivo", strlen("mandame el archivo"), 0);
    if (n < 0)
    {
        perror("fallo en enviar info");
    }

    memset(buffer, 0, TAM);

    // recibir archivo
    int fd_destino = open(archivo_destino, O_WRONLY | O_CREAT, 0666);
    if (fd_destino == -1)
    {
        perror("Error al abrir el archivo de destino");
        exit(1);
    }
    
    do
    {
      n = recv(sockfil, buffer, TAM, 0);
      if ( n < 0 ) {
	  	perror( "error de recepción\n" );
        close(fd_destino);
	  	exit(1);
	  }
      else if(n > 0){
        // Escribir los datos recibidos en el archivo de destino
            if (write(fd_destino, buffer, (size_t)n) == -1)
            {
                perror("Error al escribir en el archivo de destino");
                close(fd_destino);
                exit(1);
            }
      }

      //fwrite(buffer, sizeof(char), (size_t) n, usb);
      //size -= (size_t) n;
      size -= (size_t)n;
    }
  while(size != 0);


  char *logmd5 = md5(archivo_destino, 0);
  //printf("\nMD5 del archivo: %s\n", logmd5);
  if(strcmp(logmd5,md5_recv) == 0){
    printf("MD5 correcto %s\n",logmd5);
    printf("Archivo  descargado y MD5 verificados con exito.\n");
  }else{
    printf("ERROR: El archivo no se ha podido descargar o la integridad del mismo es incorrecta. \n");
    remove(archivo_destino);
  }


   close(fd_destino);
   close(sockfd);

   return 0;
}

void configurar_socket()
{
    memset((char *)&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons((uint16_t)puerto_files);
    struct hostent *server;
    server = gethostbyname("localhost");
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, (size_t)server->h_length);
}

/*
    Es lo mismo que connect_to_server, pero para file,
    así le pasa el archivo que quiere descargar por otro socket distinto
    al que usa para enviar comandos
*/
void conect_to_files()
{
    sockfil = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfil < 0)
    {
        perror("ERROR apertura de socket");
        exit(1);
    }

    if (connect(sockfil, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("conexion conectando");
        exit(1);
    }
    printf("conectado \n");
}
