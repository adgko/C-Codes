#include "../include/users.h"

int32_t main(void){
    struct _u_instance instance;
    int32_t fin = 0;

    //inicializa la instancia de ulfius 
    if(inicializar_server(&instance) == 1){
        print_log(LOG_USERS, "Error iniciando instancia ulfius\n");
        exit(1);
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

    ulfius_add_endpoint_by_val( instance, "GET", "/users",
                                NULL, 0, &callback_users_get, NULL);
    ulfius_add_endpoint_by_val( instance, "POST", "/users",
                                NULL, 0, &callback_users_post, NULL);

    if (ulfius_start_framework(instance) == U_OK)
      {
        char mensaje[BUFFER_SIZE];
        sprintf(mensaje, "Servidor escuchando en puerto %d\n", instance->port);
        printf("%s",mensaje);
        print_log(LOG_USERS, mensaje);
        return 0;
      }
    else
      {
        char mensaje[BUFFER_SIZE];
        sprintf(mensaje, "ERROR: iniciando framework uflius\n");
        printf("%s",mensaje);
        print_log(LOG_USERS, mensaje);
        return 1;
      }
}
//cierra y limpia el espacio de memoria de la instancia
void finalizar_server(struct _u_instance * instance)
  {
    char mensaje[BUFFER_SIZE];
    sprintf(mensaje, "Servidor cerrándose\n");
    printf("%s",mensaje);
    print_log(LOG_USERS, mensaje);
    ulfius_stop_framework(instance);
    ulfius_clean_instance(instance);
  }

#pragma GCC diagnostic ignored "-Wunused-parameter"
//Llamada a GET devuelve los users listados
// primero los obtiene del archivo en el linux
// luego los empaqueta y los devuelve empleando ulfius
int32_t callback_users_get ( const struct _u_request * request,
                struct _u_response * response, void * user_data){
    #pragma GCC diagnostic pop
    json_t *users = json_array();
    if( get_users_list(users) == 1)
      {
        return 1;
      }
    json_t *respuesta = json_pack("{s:o}", "data", users);
    ulfius_set_json_body_response( response, 200, respuesta );
    return 0;

}
//lista los usuarios y los empaqueta para ser entragados
int32_t get_users_list(json_t * body)
  {
    FILE * file = fopen(PATH_USER, "r");
    if( file == NULL)
      {
        char mensaje[BUFFER_SIZE];
        sprintf(mensaje, "ERROR: %s - %s\n", PATH_USER, strerror(errno));
        printf("%s",mensaje);
        print_log(LOG_USERS, mensaje);       
        return 1;
      }
    //recorre el archivo y guarda cada linea
    // strokea consiguiendo el id y su nombre
    char linea[BUFFER_SIZE];
    char usuario_actual[BUFFER_SIZE];
    while( fgets(linea, BUFFER_SIZE, file) != NULL )
      {
        char * aux_id;
        aux_id = strtok(linea, ":");
        sprintf(usuario_actual, "%s", aux_id);
        aux_id = strtok(NULL, ":");
        aux_id = strtok(NULL, ":");
        //cuando obtengo los datos, los voy almacenando en user
        json_t *user = json_pack("{s:s, s:s},",
                              "user_id", aux_id,
                              "username", usuario_actual
                             );
        json_array_append( body, user );

      }

    fclose(file);

    return 0;
  }

#pragma GCC diagnostic ignored "-Wunused-parameter"
//Llamada a POST ingresa un user y password
// revisa que esté bien escrita y que no haya usuarios duplicados
int32_t callback_users_post ( const struct _u_request * request,
                      struct _u_response * response, void * user_data)
  {

    #pragma GCC diagnostic pop
    json_error_t * json_error = malloc( sizeof (struct json_error_t));
    json_t * json_request = ulfius_get_json_body_request(request, json_error);

    if( request == NULL)
      {
        char mensaje[BUFFER_SIZE];
        sprintf(mensaje, "ERROR: %s\n", json_error->text);
        printf("%s",mensaje);
        print_log(LOG_USERS, mensaje);  
        free(json_error);
        return 1;
      }

    json_t * username = json_object();
    username = json_object_get(json_request, "username");

    json_t * password = json_object();
    password = json_object_get(json_request, "password");
    //corroboro que user y password sean strings y no objetos
    if( !json_is_string(username) || !json_is_string(password) || username == NULL || password == NULL)
      {
        json_t * respuesta = json_pack("{s:s}", "description",
          "La estructura de la request debe ser: "
          "'username'='<my_username>','password=<my_password>'");
        ulfius_set_json_body_response(response, 400, respuesta);
        char mensaje[BUFFER_SIZE];
        sprintf(mensaje, "Datos ingresados incorrectamente\n");
        printf("%s",mensaje);
        print_log(LOG_USERS, mensaje);  
      }
    else
      {
        char user[BUFFER_SIZE];
        sprintf(user, "%s", json_string_value(username) );
        char pass[BUFFER_SIZE];
        sprintf(pass, "%s", json_string_value(password) );
        //busco si hay usuarios con el nombre que le pasé, para no poner repetidos
        if( user_exists( user ) == 0)
          {
            json_t * respuesta = json_pack("{s:s}", "description",
                      "Usuario duplicado");
            ulfius_set_json_body_response(response, 409, respuesta);
            char mensaje[BUFFER_SIZE];
            sprintf(mensaje, "Usuario ingresado ya existe\n");
            printf("%s",mensaje);
            print_log(LOG_USERS, mensaje);
          }
        else
          {
            //agrega el usuario, con su password correspondiente
            add_user( user, pass );
            //obtengo el id de dicho usuario
            int id = get_user_id( user );
            if(id == 0)
              {
                return 1;
              }

            char tiempo[HORA_TAM];
            get_time(tiempo);
            json_t *user_json = json_pack("{s:i, s:s, s:s},",
            "id", id,
            "username", user,
            "created_at",tiempo
          );

          ulfius_set_json_body_response( response, 200, user_json );
          char mensaje[BUFFER_SIZE];
          sprintf(mensaje, "Usuario %d creado\n", id);
          printf("%s",mensaje);
          print_log(LOG_USERS, mensaje);
        }
      }

    return U_CALLBACK_CONTINUE;
  }
//pregunta si exite el usuario
// recorre el archivo, si encuentra coincidencia, lo cierra y sale
// sin cargarlo
// sino, solo vuelve, y deja que carguen normalmente
int32_t user_exists(char* nombre)
  {
    printf("esta preguntando\n");
    FILE * file = fopen(PATH_USER, "r");
    if( file == NULL)
      {
        char mensaje[BUFFER_SIZE];
        sprintf(mensaje, "ERROR: %s - %s\n", PATH_USER, strerror(errno));
        printf("%s",mensaje);
        print_log(LOG_USERS, mensaje);
        return 1;
      }

      char linea[BUFFER_SIZE];
      while( fgets(linea, BUFFER_SIZE, file) != NULL )
        {
          char * usuario = strtok(linea, ":");
          if( strcmp(usuario, nombre) == 0 )
            {
              fclose(file);
              return 0;
            }
        }

      fclose(file);

      return 1;
  }
//agrega usuario y password
// con esto, llama a system para que ejecute la acción
// y lo agregue
int32_t add_user(char* name, char* passwd)
  {
    char password[PASSWORD_TAM];
    sprintf(password, "%s", passwd);
    char nombre[PASSWORD_TAM];
    sprintf(nombre, "%s", name);
    char cmd[BUFFER_SIZE];
    sprintf(cmd,"sudo useradd %s -d /home/%s", nombre,nombre);
    //sprintf(cmd, "sudo useradd -p $(openssl passwd -1 %s) %s", password, nombre);
    system(cmd);
    return 0;
  } 
//Devuelve el id de usuario solicitado
// recorre el archivo donde están los users
// y le busca el user que matchee, y devuelve su id
int32_t get_user_id(char* nombre)
  {
    FILE * file = fopen(PATH_USER, "r");
    if( file == NULL)
      {
        char mensaje[BUFFER_SIZE];
        sprintf(mensaje, "ERROR: %s - %s\n", PATH_USER, strerror(errno));
        printf("%s",mensaje);
        print_log(LOG_USERS, mensaje);
        return 0;
      }
      int id = 0;
      char linea[BUFFER_SIZE];
      while( fgets(linea, BUFFER_SIZE, file) != NULL )
        {
          char * aux_id;
          char usuario[BUFFER_SIZE];
          aux_id = strtok(linea, ":");
          sprintf(usuario, "%s", aux_id);
          aux_id = strtok(NULL, ":");
          aux_id = strtok(NULL, ":");

          if( strcmp(usuario, nombre) == 0 )
            {
              id = (int) strtol(aux_id, NULL, 10);
              break;
            }
        }

      fclose(file);

      return id;
  }


