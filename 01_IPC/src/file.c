#include "../include/deliverymanager.h"
#define puerto_files 8020
#define IMAGES_PATH "/your/directory/log.zip"

int32_t sockfd, sock_cli;
struct sockaddr_in serv_addr;
struct sockaddr_in client_addr;
uint32_t client_len; // tamaño de la dirección del cliente
char buffer[TAM];
ssize_t n;	   // hubo que declarar n como ssize_t para que no pierda información al usarse en send() y recv()
long *logsize; // para guardar size de log
char *logmd5;  // para guardar md5 de log
int32_t main()
{

	configurar_socket(); // configura y abre el socket para transmitir

	escuchando(); // espera que se conecte un socket

	// En el servidor (file)
	printf("Esperando conexiones en el puerto %d\n", puerto_files);

	while (1)
	{
		conectar_cliente();

		// char *mensaje = "conexión";

		n = send(sock_cli, "conexion", strlen("conexion"), 0);
		if (n < 0)
		{
			perror("fallo en enviar info");
		}

		 logsize=calloc(1,sizeof(long));
		 logmd5=calloc(TAM,sizeof(char));

		 read_log();

		 char * logsizeChar=calloc(TAM,sizeof(char)); 
		 sprintf(logsizeChar,"%ld",*logsize);
		 printf("tamanio %s\n",logsizeChar);
		 printf("hash md5: %s\n",logmd5);

		// envía tamaño
		 n = send(sock_cli, logsizeChar , strlen(logsizeChar), 0);
		if (n < 0)
		{
			perror("fallo en enviar info");
		}
		// espera confirmaciòn
	    n = recv(sock_cli, buffer, TAM - 1, 0);
		if (n < 0)
		{
			perror("lectura de socket");
			exit(1);
		}

		// envía  hash md5
		 n = send(sock_cli, logmd5 , strlen(logmd5), 0);
		if (n < 0)
		{
			perror("fallo en enviar info");
		}

		// espera confirmaciòn
	    n = recv(sock_cli, buffer, TAM - 1, 0);
		if (n < 0)
		{
			perror("lectura de socket");
			exit(1);
		}

		// enviar el archivo
		enviar_log();

		 free(logsize);
		 free(logmd5);
		 free(logsizeChar);

		// exit(0);
	}
	return 0;
}

/*
	Configura el socket con el que se enviará la imagen
*/
void configurar_socket()
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	memset((char *)&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons((uint16_t)puerto_files);
	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("conexion");
		exit(1);
	}
}

/*
	Abre conexión para que se conecte un cliente
*/
void escuchando()
{
	listen(sockfd, 1);
	client_len = sizeof(client_addr);
}

/*
	Busca el archivo solicitado, avisa si lo encontró o no, y en caso afirmativo
	envía dicho archivo
*/
void read_log()
{

	char img[TAM];
	long size = 0;

	sprintf(img, "%s", IMAGES_PATH); // Path de la imagen

	FILE *imgn;
	imgn = fopen(img, "r");
	if (imgn == NULL)
	{
		perror("file");
		return;
	}

	// Calcula el tamaño de la imagen
	fseek(imgn, 0, SEEK_END);
	size = ftell(imgn);
	fclose(imgn);

	// guarda tamaño para enviar
	// char size_s[TAM] = "";
	logsize = malloc(sizeof(long));
    if (logsize == NULL) {
        perror("error al reservar memoria para logsize");
        //exit(EXIT_FAILURE);
    }
	*logsize = size;
	logmd5 = md5(img, 0);
}

/*
	Conecta con el cliente que usó su dirección
*/
void conectar_cliente()
{

	sock_cli = accept(sockfd, NULL, NULL);
	if (sock_cli < 0)
	{

		perror("accept");
		exit(1);
	}
	printf("conectado\n");
}

void enviar_log() {
    char img[TAM];

    sprintf(img, "%s", IMAGES_PATH); // Path de la imagen
    FILE *imgn = fopen(img, "r");
    if (imgn == NULL) {
        perror("Error al abrir el archivo");
        exit(1);
    }

    // Obtener el tamaño del archivo
    fseek(imgn, 0, SEEK_END);
    //long size = ftell(imgn);
    fseek(imgn, 0, SEEK_SET);

    // Leer y enviar el archivo en fragmentos
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, TAM, imgn)) > 0) {
        if (send(sock_cli, buffer, bytes_read, 0) == -1) {
            perror("Error al enviar el archivo");
            exit(1);
        }
    }

    fclose(imgn);
}

