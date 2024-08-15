#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>
#include <math.h>
#include <time.h>
#include <omp.h>
#include <string.h>
#include "simple_bmp.h"


void validar_colores(double* a,double* b,double* c);
void validar(double* x);
void normalizar_colores(double* a,double* b,double* c);
void normalizar(double *z);
//void procesamiento(double* R,double* B,double* G,sbmp_image image,int32_t x_size);
void procesamiento(sbmp_image image);