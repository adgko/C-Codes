#include "../include/simple_bmp.h"
#include "../include/memoria_compartida.h"

#define FILE_PATH "../archivos/origen/OR_ABI-L2-MCMIPF-M6_G16_s20202761400193_e20202761409513_c20202761409596.nc"
#define IMAGEN_FINAL "../archivos/destino/salida.bmp"
#define NDIMS 2
#define X_SIZE 5424 
#define Y_SIZE 5424
//#define K       273
//#define tr      333
//#define uno     1
//#define zero    0
//#define corr    0.75
//#define gamma   0.4

      double B[X_SIZE][Y_SIZE];
      double G[X_SIZE][Y_SIZE];
      double R[X_SIZE][Y_SIZE];

int num_threads;

void ncError(int val)
{
  printf("Error: %s\n", nc_strerror(val));
  exit(2);
}



int32_t main(int32_t arg, char *argv[]){

    if(arg < 2){}
     //variables para NetCDF
    int32_t ncid;
    //int32_t* x_size,y_size;    
    int32_t varid5,varid6,varid7,ptr;                       
    int32_t status = NC_NOERR;
    // el centro de la imagen y el radio para trabajar zonas
    //int32_t centro[] = {Y_SIZE/2, X_SIZE/2};          
    double radio = X_SIZE/2;
    radio = pow(radio,2);
    //número de threads que usa el programa
    num_threads = atoi(argv[1]);
    omp_set_num_threads(num_threads);

	 // para saber cuanto dura la función
	  double start, end;
    double cpu_time_used;
//    start = omp_get_wtime();


    
    //Abrir el archivo y obtener un id para trabajarlo
    status = nc_open(FILE_PATH,NC_NOWRITE,&ncid); //se pone en NC_NOWRITE para que no sobvreescriba el archivo
    if(status != NC_NOERR) ncError(status);

    if((status = nc_inq_dimid(ncid,"y",&ptr))) ncError(status);
   // if((status = nc_inq_dimlen(ncid,ptr,Y_SIZE))) ncError(status);
    if((status = nc_inq_dimid(ncid,"x",&ptr))) ncError(status);
   // if((status = nc_inq_dimlen(ncid,ptr,X_SIZE))) ncError(status);

    //incialización de matrices para los canales
//    double *B = calloc(1,sizeof(double)*X_SIZE*Y_SIZE);
//    double *G = calloc(1,sizeof(double)*X_SIZE*Y_SIZE);
//    double *R = calloc(1,sizeof(double)*X_SIZE*Y_SIZE);



    //Almaceno las matrices de los colores para ser modificadas
    if ((status = nc_inq_varid(ncid, "CMI_C05", &varid5))) ncError(status);      	
    if ((status = nc_inq_varid(ncid, "CMI_C06", &varid6))) ncError(status);  
    if ((status = nc_inq_varid(ncid, "CMI_C07", &varid7))) ncError(status);      	
    
    // Leemos las matrices y guardamos sus números
//	  if ((status = nc_get_var_double(ncid, varid5, B))) ncError(status); 
//    if ((status = nc_get_var_double(ncid, varid6, G))) ncError(status);
//    if ((status = nc_get_var_double(ncid, varid7, R))) ncError(status);

    if ((status = nc_get_var_double(ncid, varid5, &B[0][0]))) ncError(status);
    if ((status = nc_get_var_double(ncid, varid6, &G[0][0]))) ncError(status);
    if ((status = nc_get_var_double(ncid, varid7, &R[0][0]))) ncError(status);

    //inicializamos una imagen tipo bmp
    sbmp_image salida = {0};
    if(sbmp_initialize_bmp(&salida,X_SIZE,Y_SIZE) == -1){
      perror("Fallo en inicializar imagen\n");
    }		
    //procesa la imagen y la guarda 
//    procesamiento(B,G,R,salida,X_SIZE);
    start = omp_get_wtime();
    procesamiento(salida);
    end = omp_get_wtime();
    cpu_time_used = ((double) (end - start));

    //Guardo la imagen
    sbmp_save_bmp(IMAGEN_FINAL,&salida);

    //Cuando se termina de usar, cierra el archivo
    status = nc_close(ncid);
    if(status != NC_NOERR) ncError(status);

    // para calcular cuanto dura                                                   
//	  end = omp_get_wtime();
//    cpu_time_used = ((double) (end - start));
    printf("Tiempo empleado: %f milisegundos\n",(float)cpu_time_used*1000);
}
//Ingresan los 3 colores, y llama a la función para cada uno
void validar_colores(double* a,double* b,double* c){
  validar(a);
  validar(b);
  validar(c);
}
//Revisa que los colores tengan valores entre números válidos
void validar(double* x){
	if(*x > 255){
		*x = 255;
	}
	else if(*x < 0){
		*x = 0;
  }
}
//Ingresan los 3 colores, y llama a la función para cada uno
void normalizar_colores(double* a,double* b,double* c){
  normalizar(a);
  normalizar(b);
  normalizar(c);
}
//Revisa que los colores tengan valores entre números válidos
void normalizar(double *z){
	if(*z > 1){
		*z = 1.0;
	}
	else if(*z < 0){
		*z = 0;
	}
}


//void procesamiento(double* R,double* B,double* G,sbmp_image salida,int32_t x_size){
void procesamiento(sbmp_image salida){

      // declaraciones para bloques paralelos
    double scale05 = 0.00031746;
    double scale06 = 0.00031746;
    double scale07 = 0.01309618;
    double offset05 = 0;
    double offset06 = 0;
    double offset07 = 197.31;
    double k = 273;
    double tr = 333;
    double uno = 1;
    double zero = 0;
    double corr = 0.75;
    double gamma = 0.4;

    #pragma omp parallel
    {

    //int id = omp_get_thread_num();
    // En cada canal ocurre lo siguiente:
    // 1. Normalizado de canales
    // 2. Validar colores
    // 3. Aplicar corrección gamma al canal rojo
    // se paraleliza por fila, recibiendo una cada thread por vez
    #pragma omp for collapse(2)
    for(int32_t i=0; i<X_SIZE; i++){
      for(int32_t j=0; j<Y_SIZE; j++){


       // if(((i-centro[0])*(i-centro[0])+(j-centro[1])*(j-centro[1])) <= radio){
        //almaceno todos los valores en varibles auxiliares
//        double blue_aux    = (double)(B[i*x_size+j]);
//        double green_aux   = (double)(G[i*x_size+j]);  
//        double red_aux     = (double)(R[i*x_size+j]);

        double blue_aux = B[i][j];
        double green_aux = G[i][j];
        double red_aux = R[i][j];


        // canal rojo
        if(red_aux != -1){
            red_aux = red_aux * scale07;
          //  printf("%f\n",red_aux);
            
            red_aux = red_aux+ offset07; //corrijo valor
          //  printf("%f\n",red_aux);
            red_aux = ((red_aux - k) / (tr - k)); //normalizo

            if(red_aux > 1){
                red_aux = pow(red_aux, (1/gamma)); //aplico corrección gamma
            }
            else if(red_aux > 0){
                red_aux = pow(red_aux, (1/gamma)); //aplico corrección gamma
            }
            else{
                red_aux = 0;
            }
        }

        // canal verde
        if(green_aux != -1){
            green_aux = green_aux * scale06 + offset06; //corrijo valor
            green_aux = ((green_aux - zero) / (uno - zero)); //normalizo
            //printf("%f\n",green_aux);
        }

        // canal azul
        if(blue_aux != -1){
            blue_aux = blue_aux * scale05 + offset05; //corrijo valor
            blue_aux = ((blue_aux - zero) / (corr - zero)); //normalizo
        }



      //printf("%f %f %f\n",red_aux,green_aux,blue_aux);
      //printf("%f\n",green_aux);

       //normalizo los colores
       normalizar_colores(&red_aux,&blue_aux,&green_aux);

        //valido los colores para que se queden entre 0 y 255
      validar_colores(&red_aux,&blue_aux,&green_aux);

        //printf("%f\n",green_aux);

        //obtengo el número más pequeño o igual a la variable, para 
        // evitar que genere un core
        red_aux = (double)floor(red_aux*255);
        blue_aux = (double)floor(blue_aux*255);
        green_aux = (double)floor(green_aux*255);

        //vuelvo a guardar las variables modificadas en la matriz
       // R[j*X_SIZE+i] = red_aux;
       // B[j*X_SIZE+i] = blue_aux;
       // G[j*X_SIZE+i] = green_aux;

        //printf("%f %f %f\n",red_aux,green_aux,blue_aux);


        //Almaceno los valores en los canales de la matriz, 
        // de la imagen bmp que voy a generar
        salida.data[i][j].red   = (u_int8_t)(red_aux);
        salida.data[i][j].blue  = (u_int8_t)(blue_aux);
        salida.data[i][j].green = (u_int8_t)(green_aux);
    //   }
      }
    }
    }
}
