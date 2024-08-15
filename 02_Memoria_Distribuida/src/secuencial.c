#include "../include/simple_bmp.h"
#include "../include/memoria_compartida.h"

#define FILE_PATH "../archivos/origen/OR_ABI-L2-MCMIPC-M6_G16_s20202761516159_e20202761518532_c20202761519087.nc"
#define IMAGEN_FINAL "../archivos/destino/salida.bmp"
#define NDIMS 2
#define X_SIZE 2500 
#define Y_SIZE 1500
#define K       273
#define tr      333
#define uno     1
#define zero    0
#define corr    0.75
#define gamma   0.4

void ncError(int val)
{
  printf("Error: %s\n", nc_strerror(val));
  exit(2);
}



int32_t main(){

    int32_t ncid, ndims, nvars, ngatts, unlimdimid;
    int32_t varid5,varid6,varid7;
    int32_t status = NC_NOERR;
    //int32_t centro[] = {Y_SIZE/2, X_SIZE/2};
    double radio = X_SIZE/2;
    radio = pow(radio,2);

    //float scale05 = 0.00031746f;
    //float scale06 = 0.00031746f;
    //float scale07 = 0.01309618f;

    //float offset05 = 0.f;
    //float offset06 = 0.f;
    //float offset07 = 197.31f;

	  // para saber cuanto dura la función
	  double start, end;
    double cpu_time_used;
    start = omp_get_wtime();

    short *B = calloc(1,sizeof(int32_t)*X_SIZE*Y_SIZE);
    short *G = calloc(1,sizeof(int32_t)*X_SIZE*Y_SIZE);
    short *R = calloc(1,sizeof(int32_t)*X_SIZE*Y_SIZE);

    

    status = nc_open(FILE_PATH,NC_WRITE,&ncid);
    if(status != NC_NOERR) ncError(status);

    status = nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid);
    if (status != NC_NOERR) ncError(status);

       if (ndims != 5 || nvars != 161 || ngatts != 29 ||
       unlimdimid != -1) return 2;

    // imprimo un par de valores. Estaba probando las funciones. En la versión final no van
    printf("ID: %d\n",ncid);
    printf("Numero de dimensiones: %d\n",ndims);
    printf("Numero de variables %d\n",nvars);
    printf("Numero de atributos globales %d\n",ngatts);
    printf("Dimensiones ilimitadas %d\n",unlimdimid);

    //Almaceno las matrices de los colores para ser modificadas
    if ((status = nc_inq_varid(ncid, "CMI_C05", &varid5))) ncError(status);      	
    if ((status = nc_inq_varid(ncid, "CMI_C06", &varid6))) ncError(status);  
    if ((status = nc_inq_varid(ncid, "CMI_C07", &varid7))) ncError(status);      	
    
    	// Leemos las matrices
	  if ((status = nc_get_var_short(ncid, varid5, B))) ncError(status); 
    if ((status = nc_get_var_short(ncid, varid6, G))) ncError(status);
    if ((status = nc_get_var_short(ncid, varid7, R))) ncError(status);  		

    // En cadacanal ocurre lo siguiente:
    // 1. Normalizado de canales
    // 2. Validar colores
    // 3. Aplicar corrección gamma al canal rojo
    for(int i=0; i<X_SIZE; i++){
      for(int j=0; j<Y_SIZE; j++){

       // if(((i-centro[0])*(i-centro[0])+(j-centro[1])*(j-centro[1])) <= radio){
        //almaceno todos los valores en varibles auxiliares
        //short blue_aux  = (short)(B[i*X_SIZE+j]*scale05+offset05);
        //short green_aux = (short)(G[i*X_SIZE+j]*scale06+offset06);
        //short red_aux   = (short)(R[i*X_SIZE+j]*scale07+offset07);
        short blue_aux    = (short)(B[i*X_SIZE+j]);
        short green_aux   = (short)(G[i*X_SIZE+j]);  
        short red_aux     = (short)(R[i*X_SIZE+j]);

        //normalizo siguiendo las indicaciones
        red_aux =  (short)((red_aux-K)/(tr-K));
        blue_aux = (blue_aux-zero)/(uno-zero);
        green_aux = (short)((green_aux-zero)/(corr-zero));

        //valido los colores para que se queden entre 0 y 1
        validar_colores(&red_aux,&blue_aux,&green_aux);

        //aplico corrección gamma en el canal rojo
        red_aux = (short)pow(red_aux,(1/gamma));

        //vuelvo a guardar las variables modificadas en la matriz
        R[i*X_SIZE+j] = red_aux;
        B[i*X_SIZE+j] = blue_aux;
        G[i*X_SIZE+j] = green_aux;
    //   }
      }
    }

    sbmp_image salida = {0};
    if(sbmp_initialize_bmp(&salida,X_SIZE,Y_SIZE) == -1){
      perror("Fallo en inicializar imagen\n");
    }
    for(int i=0; i<X_SIZE ; i++){
      for(int j = 0; j < Y_SIZE ; j++){
     //   if(((i-centro[0])*(i-centro[0])+(j-centro[1])*(j-centro[1])) <= radio){
        int32_t red_aux   = (int32_t)floor(R[i*X_SIZE+j]);
        int32_t blue_aux  = (int32_t)floor(B[i*X_SIZE+j]);
        int32_t green_aux = (int32_t)floor(G[i*X_SIZE+j]);

//        normalizar_colores(&red_aux,&blue_aux,&green_aux);

        salida.data[i][j].red   = (u_int8_t)(red_aux*500);
        salida.data[i][j].blue  = (u_int8_t)(blue_aux*500);
        salida.data[i][j].green = (u_int8_t)(green_aux*500);
      //  }
      }
    }

    sbmp_save_bmp(IMAGEN_FINAL,&salida);

    //Cuando se termina de usar, cierra el archivo
    status = nc_close(ncid);
    if(status != NC_NOERR) ncError(status);

    // para calcular cuanto dura
	  end = omp_get_wtime();
    cpu_time_used = ((double) (end - start));
	  int h,m,s;
	  h = (int)(cpu_time_used/3600);
	  m = (int)(cpu_time_used -(3600*h))/60;
	  s = (int)(cpu_time_used -(3600*h)-(m*60));
	  //printf("%sProcesamiento Completado%s\n",KGRN,KNRM );
	  //printf("%sLa operación duró %d horas, %d minutos y %d segundos %s\n", KGRN, h, m, s, KNRM);
    printf("La operación duró %d horas, %d minutos y %d segundos \n", h, m, s);

}

void validar_colores(short* a,short* b,short* c){
  validar(a);
  validar(b);
  validar(c);
}

void validar(short* x){
	if(*x > 255){
		*x = 255;
	}
	else if(*x < 0){
		*x = 0;
  }
}

void normalizar_colores(int32_t* a,int32_t* b,int32_t* c){
  normalizar(a);
  normalizar(b);
  normalizar(c);
}

void normalizar(int32_t *z){
	if(*z > 1){
		*z = 1;
	}
	else if(*z < 0){
		*z = 0;
	}
}