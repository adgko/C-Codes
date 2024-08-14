#include "../include/cliente.h"

int32_t sockfd, sockfil,puerto;
struct sockaddr_in serv_addr,serv_addr_file;

void signal_handler(){
	printf("cerrando conexión\n");
	close(sockfd);
	exit(1);
}

/*
	Función que se conecta al Delivery Managment vía socket y recibe mensajes
*/

int32_t main( int argc, char *argv[] ) {
	
	
	struct hostent *server;
    ssize_t n;
	//ssize_t size;

	char buffer[TAM];
	if ( argc < 3 ) {
		fprintf( stderr, "Uso %s host puerto\n", argv[0]);
		exit( 0 );
	}

	puerto = atoi( argv[2] );
	sockfd = socket( AF_INET, SOCK_STREAM, 0 );


	server = gethostbyname( argv[1] );

	memset( (char *) &serv_addr, '0', sizeof(serv_addr) );
	serv_addr.sin_family = AF_INET;
	serv_addr_file.sin_family = AF_INET;	// que se conecte al socket de log
	bcopy( (char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, (size_t )server->h_length );
	bcopy( (char *)server->h_addr, (char *)&serv_addr_file.sin_addr.s_addr, (size_t)server->h_length ); //a file
	serv_addr.sin_port = htons( (uint16_t)puerto );
	serv_addr_file.sin_port = htons( (uint16_t) puerto_files );											 // a file
	if ( connect( sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr ) ) < 0 ) {
		perror( "conexion" );
		exit( 1 );
	}

	if (signal(SIGINT, signal_handler) == SIG_ERR) {			// crea el signal, el cual llamara al handler en caso de ingresar Ctrl+C
        fputs("Error al levantar el handler.\n", stderr);
        return EXIT_FAILURE;
    }

		while(1) {
			memset( buffer, 0, TAM );
			
			n = read( sockfd, buffer, TAM-1 );
			if ( n < 0 ) {
			  perror( "lectura de socket" );
			  exit(1);
			}
			
			char mensaje[strlen(buffer)];
			sprintf(mensaje,"%s",buffer);
			printf("El mensaje mensaje es: %s\n",mensaje);

			char* token;
			token = strtok(buffer, " ");
			char hash[1024];
			sprintf(hash, "%s", token);
			//printf("%s\n", hash);

			token = strtok(NULL, "\n");
			char mensaje_aux[1024];
			sprintf(mensaje_aux, "%s", token);
			//printf("%s\n", mensaje_aux);

			//char* hashmd5 = md5(mensaje_aux,(int)strlen(mensaje_aux));

			//printf("%s %s\n",hash,hashmd5);
			
			//if(strcmp(hash,hashmd5) == 0){
				printf( "%sRecibí: %s%s\n", KBLU,mensaje_aux,KNRM );

			if(strcmp(mensaje_aux,"download-file") == 0){

				int32_t downloader = fork();
				if ( downloader == 0 ) {
					char *args[] = {DOWNLOAD, argv[1], argv[2], NULL};
					if( execv(DOWNLOAD, args) == -1 ) {
					perror("error en downloader ");
					//exit(1);
					} 			
					//exit(0);
				}

			}
			//}
			else{
			n = write( sockfd, "Obtuve su mensaje", 18 );
			if ( n < 0 ) {
				perror( "escritura en socket" );
				exit( 1 );
			}
			}
			// Verificación de si hay que terminar
			buffer[strlen(buffer)-1] = '\0';
			if( !strcmp( "fin", buffer ) ) {
			  printf( "PROCESO %d. Como recibí 'fin', termino la ejecución.\n\n", getpid() );
			  exit(0);
			}

		}

	return 0;
} 

/*
	Es lo mismo que connect_to_server, pero para file, 
	así le pasa el archivo que quiere descargar por otro socket distinto
	al que usa para enviar comandos
*/
void conect_to_files(){
	sockfil = socket( AF_INET, SOCK_STREAM, 0 );
	if ( sockfil < 0 ) {
		perror( "ERROR apertura de socket" );
		exit( 1 );
	}

	if ( connect( sockfil, (struct sockaddr *)&serv_addr_file, sizeof(serv_addr_file ) ) < 0 ) {
		perror( "conexion" );
		exit( 1 );
	}
	printf("conectado \n");
}
