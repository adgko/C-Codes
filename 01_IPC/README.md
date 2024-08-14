# IPC
Este programa fue una consigna de la materia Sistemas Operativos 2 para evaluar el uso de IPC en Lenguaje C.

El requerimiento era generar un socket Servidor al que se conectaran sockets Clientes, el Servidor es un Delivery Manager usando el patrón [Publisher Subscriber](https://en.wikipedia.org/wiki/Publish%E2%80%93subscribe_pattern "Publisher Subscriber") que tiene de su lado procesos que funcionan como productores y puede enviar mensajes producidos por dichos productores a los clientes seleccionados.

## Instalación
Para instalar este proyecto, se debe tener instalada la biblioteca de zip que se encuentra en [este](https://github.com/kuba--/zip) repo público y usar el archivo de makefile que viene en el mismo.

`make`

## Requerimientos

### Delivery Manager
- El Delivery Manager debe poder recibir los mensajes de los productores y reenviarlos a los clientes que corresponda.
- Debe tener una interfaz CLI (Command Line Interface) que acepta únicamente los siguientes comandos
	- `add <socket> <productor>` : Este comando agregara el socket a una lista correspondiente al servicio, para ser validado.
	`delete <socket> <productor>`: Este comando borra el host como suscriptor dejando de enviarle mensajes.
	`log <socket>`. Este comando comprime el log local del _DeliveryManager_ y lo envía a `<socket>`.
-	El _DeliveryManager_ debe validar que los hosts agregados sean aptos para recibir mensaje del tipo `<Mensaje>|<Checksum>`.	
-	Una vez validado el host, todos los mensajes que recibe el _DeliveryManager_ de un productor, deben ser enviados a los suscriptores correspondientes.
-	El _DeliveryManager_ debe loguear todos los mensajes enviados, tanto el origen como el destino, también se agrega o se elimina un suscriptor y cuando es validado. El formato del log debe `<datetime> <Mensaje>`. 

### Productores
Se deben implementar tres productores:
- Un productor que envía un mensaje random con una tasa de X/segundos.
- Un productor que envía la memoria libre del Sistema, cada Y/segundos.
- Un productor que debe enviar load del sistema normalizado, cada Z/segundos.
Pueden elegir otro productor, justificándolo. X, Y y Z deben ser distintos.

### Suscriptores
- Puede existir hasta mil suscriptores, y deben ser capaz de vivir en una misma instancia.
- Los suscriptores deben esperar que el _DeliveryManager_ los suscriba mediante el comando `add`, es decir, que lo suscriba.
- Los suscriptores deben validar el `checksum` de los mensajes recibidos y loguear el mensaje, para luego ser descartados.


### Restricciones
El diseño debe contemplar toda situación no descripta en el presente documento y se debe hacer un correcto manejo de errores. 

## Desarrollo

Diagrama del Programa:
![](https://github.com/adgko/C-Codes-/blob/main/01_IPC/img/Diagrama%20de%20Sistema.jpg)

### Delivery Manager
Para el desarrollo del Delivery Manager se trabajó en los siguientes puntos.

#### Configuración de Socket
El Socket Servidor se configuró como socket de internet, para funcionar con ip y puerto, para atender las conexiones.

`sockfd = socket(AF_INET, SOCK_STREAM, 0);`

Y prepararse para escuhcar hasta 5000 conexiones

`listen(sockfd, MAX_CLIENTES)`

La atención de las distintas conexiones se hace mediante epoll(), que permite ir evaluando cada conexión mediante su file descriptor fd. Si es un cliente, se lo atiende en conexión mientras no supere los 5000. Si se trata del mismo server, atiende las operaciones de comunicaciones descriptas más abajo.

#### Handler
Se escribió un handler en caso de recibir una interrupción de sistema. Este envía a una subrutina que cierra las conexiones del Delivery con los clientes y cierra la cola de mensaje.

#### Cola de Mensajes
Para la conexión entre los productores, CLI y Delivery Manager, se empleó Cola de Mensajes. Se maneja con las funciones provista por la biblioteca `<sys/ipc.h>`

La forma de funcionamiento consiste en que el CLI se comporta igual que los productores, en cuanto a que solo agregan mensajes a la cola de mensajes, con una etiqueta. Del lado Delivery Manager se fija si hay mensaje con la etiqueta solicitada, si es así los saca para realizar operaciones, sino continúa sin bloquearse.

Es importante que al cerrar el programa se cierre la cola de mensaje, por lo que la subrutina a la que lleva el handler también tiene la función de cerrar la cola de mensajes.

#### Productores
Para el manejo de los productores, se los crea como procesos hijos. 

	
	pid_prod1 = fork();
		if (pid_prod1 == 0)
		{
			if (execv(PROD1_PATH, argv) == -1)
			{
				perror("error en productor 1 ");
				exit(1);
			}
			exit(0);
		}

Se hicieron 3 procesos hijos productores.
 - El primero genera un número random empleando la función rand() y lo envía al Delivery Manager cada 1/X segundos.
 - El segundo revisa cuanta memoria libre le queda al sistema y envía el dato al Delivery Manager cada 1/Y segundos. Para consultar ese dato, obtiene la información de `/proc/meminfo`
 - El tercero obtiene el load normalizado del sistema y lo envía al Delivery Manager cada 1/Z segundos. Para conseguir ese dato se vale de la función `getloadavg()`
 
#### Manejo de Sockets Clientes
Los socket clientes se conectando cuando Delivery Manager ejecuta `accept()` Luego de eso, tiene una estructura Lista donde guarda los clientes con su file descriptor, ip, puerto, y almacena flags para subscribirlo a los productores.

	struct lista { /* lista simple enlazada */
	  int32_t fd;
	  char* ip;
	  int32_t port;
	  int subs_1;
	  int subs_2;
	  int subs_3;
	  struct lista *sig;
	};

Esta estructura es empleada tanto para la subscripción y desuscripción, como para el envío de mensajes y logs.
 
#### Manejo de Mensajes
Cuando el Delivery encuentra un mensaje en la Cola de Mensajes con una etiqueta de Productor 1, 2 o 3, lo que hace es leerlo, calcularle el hash md5, y agregar ambos a un buffer. Luego el contenido de ese buffer es enviado a cada cliente que haya sido subscrito a dicho productor. El delivery manager recorre la lista de sockets clientes y cuando la flag de subcripción coincide, le envía el dato.
Para el calculo de hash md5 se emplea funciones escritas en el archivo `funciones1.h` que emplean la biblioteca `<openssl/md5.h>`.

#### CLI
El CLI es un proceso hijo que ejecuta el Delivery Manager al comienzo del proceso. Está pendiente de lo que ingresan por teclado y envía la orden al Delivery Manager por cola de mensaje.
Cuando el Delivery Manager la recibe, la procesa separandola en `comando`,`parámetro` y `argumento`.

Dependiendo lo que ingresa, es lo que ocurre.
 - Comando `add`busca en la lista de conectados el que coincida con la ip y puerto suministrada en `parámetro` y le cambia la flag de suscripto a `1` que indique el `argumento` (1, 2 o 3).
 - Comando `delete` busca en la lista de conectados el que coincida con la ip y puerto suministrada en `parámetro` y le cambia la flag de suscripto a `0` que indique el `argumento` (1, 2 o 3).
 - Comando `log` busca en la lista de conectados el que coincida con la ip y puerto suministrada en `parámetro` y le envía un mensaje de descarga. De ahí se encarga el proceso `file`.
 
### Socket Cliente
El socket Cliente no es muy complejo realmente. Tiene la configuración de socket de internet para recibir por parámetro la ip y puerto al que debe conectarse. 
Una vez conectado al Delivery Manager, solo espera la llegada de mensajes, a los cuales separa en dos partes, la primera es el hash md5 enviado y la segunda el mensaje propiamente dicho. Calcula el hash del mensaje y lo compara con lo primero que llegó para saber si es correcto. En ese caso, lo imprime en pantalla y nada más.

Además, el cliente valúa los mensajes para ver si es la orden de download. En ese caso, abre un proceso hijo que consiste en un nuevo socket que se conectará a `File` y recibirá un archivo.

### Envío de Log
Para hacer este proceso, se armaron dos procesos apartes. File es invocado por Delivery Manager cuando arranca y estará siempre corriendo esperando una conexión de Downloader. 
El proceso consiste en que CLI envía un mensaje de log ocon la ip y puerto de un socket cliente. Delivery Manager llama a la función `zipear()` construida con la biblioteca zip obtenida de este [repo](https://github.com/kuba--/zip). Luego envía el mensaje de descarga al cliente y continúa con su tarea.
Downloader por otro lado, es creado por un socket cliente que recibe una descarga, este se conecta con File, negocia y recibe el log, y se cierra.
El funcionamiento se puede ver en el siguiente diagrama.

![](https://github.com/adgko/C-Codes-/blob/main/01_IPC/img/DiagramaArchivo.png)

#### File
El código de File consiste en configurar el socket y esperar conexiones. Cuando se conecta un cliente, abre el log, obtiene su tamaño y hash md5, y lo envía al Downloader, para luego enviar el archivo. Al terinar, se queda esperando nuevas conexiones.

#### Downloader
Este código tiene una parte de configurar el socket y conectarse a File. Una vez conectado, recibe tamaño de archivo, confirma, luego recibe hash md5, confirma, y recibe el archivo, empleando el tamaño recibido para saber cuanto debe recibir. Cuando recibe el archivo, le calcula el hash md5 y lo comparo con lo que recibió para corroborar que no se haya corrompido en el envío.  Si el archivo está validado, envía una confirmación, de lo contrario, lo elimina y avisa que no se transfirió con éxito. Al finalizar, cierra la conexión y se cierra el proceso.
