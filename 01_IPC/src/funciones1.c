#include "../include/funciones1.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                               COLA DE MENSAJE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	crea la cola si no existe y devuelve el id
*/
int32_t get_queue() {

  key_t qkey;
  qkey = ftok(QUEUE_NAME, UNIQUE_KEY);		//devuelve una key desde el path y los 8 bits menos significativos de PROJ_ID

  if (qkey == -1) {
    perror("error obteniendo token");
    exit(1);
  }

  return msgget(qkey, PROJ_ID|IPC_CREAT);				//devuelve el id de la cola asociada a qkey. IPC_CREATE sirve para que falle si la cola ya existe
}

/*
	Se emplea para enviar los mensajes a la cola, 
	con la que se comunica server, file y auth
*/
int32_t send_to_queue(long id_mensaje, char mensaje[TAM]) {

	int32_t msqid=get_queue();
  if(strlen(mensaje) > TAM) {
    perror("error, mensaje muy grande\n");
    exit(1);
  }

  struct msgbuf mensaje_str;
  mensaje_str.mtype = id_mensaje;
  sprintf(mensaje_str.mtext, "%s", mensaje);

  return msgsnd(msqid, &mensaje_str, strlen(mensaje_str.mtext) + 1, 0 );
}

char* recive_from_queue(long id_mensaje, int32_t flag) {
  
  int32_t msqid=get_queue();
  errno = 0;			//seteo en 0 para que no se pise cada vez que lo llamo
  struct msgbuf mensaje_str = {id_mensaje, {0}};
  
  if(msgrcv(msqid, &mensaje_str, sizeof mensaje_str.mtext, id_mensaje, flag) == -1) {
      if(errno != ENOMSG) {
        perror("error recibiendo mensaje de cola");
        exit(1);
      }
  }
  char* mensaje = malloc(strlen(mensaje_str.mtext)+1);
   if (mensaje == NULL) {
        perror("error asignando memoria");
        exit(1);
    }
  sprintf(mensaje,"%s", mensaje_str.mtext);
  return mensaje;
}

// Función para vaciar una cola de mensajes
/*void vaciar_cola() {
    struct msgbuf msg;
    int32_t msqid=get_queue();
    // Vaciar la cola de mensajes
    while (msgrcv(msqid, &msg, MAX_MESG_SIZE, 0, IPC_NOWAIT) != -1) {
        printf("Mensaje vaciado: %s\n", msg.mtext);
    }
    if (errno != ENOMSG) {
        perror("Error al vaciar la cola de mensajes");
    }
}
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//								                          MD5
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	Función que calcula el MD5 de un mensaje
  Le pasas a la función el char del mensaje y su tamaño y te devuelve el hash

  La función  la tomé de https://github.com/cjtrowbridge/md5-crack-2/blob y la adapté 
*/

char *md5(const char *str, int length) {
    int n;
    MD5_CTX c;
    unsigned char digest[MD5_DIGEST_LENGTH];
    char *out = (char*)malloc(MD5_DIGEST_LENGTH * 2 + 1);

    MD5_Init(&c);

    while (length > 0) {
        if (length > 512) {
            MD5_Update(&c, str, 512);
        } else {
            MD5_Update(&c, str, (size_t)length);
        }
        length -= 512;
        str += 512;
    }

    MD5_Final(digest, &c);

    for (n = 0; n < MD5_DIGEST_LENGTH; ++n) {
        snprintf(&(out[n*2]), MD5_DIGEST_LENGTH*2, "%02x", (unsigned int)digest[n]);
    }

    out[MD5_DIGEST_LENGTH * 2] = '\0';

    return out;
}
