#include <stdio.h>
#include <stdlib.h>
#include <ulfius.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include "../include/funciones.h"

#define PUERTO 9013
#define BUFFER_SIZE 1024
#define PASSWORD_TAM 20
#define PATH_USER "/etc/passwd"
#define LOG_USERS 1

int32_t inicializar_server(struct _u_instance*);
void finalizar_server(struct _u_instance*);
int callback_users_get(const struct _u_request*, struct _u_response*, void*);
int32_t get_users_list(json_t*);
int callback_users_post(const struct _u_request*, struct _u_response*, void*);
int user_exists(char*);
int add_user(char*, char*);
int get_user_id(char*);
void get_time(char*);
