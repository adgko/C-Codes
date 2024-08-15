# Memoria Distribuída
El siguiente proyecto se compone de dos programas. Ambos realizan la misma función, pero uno lo hace de manera secuencial y otro empleando libreria OpenMP y código paralelo, logrando una mejoría en el tiempo.
Este trabajo se presentó como consigna para la materia Sistemas Operativos 2 y se detalla a continuación los requerimientos que nos dieron.

## Requerimientos de la Cátedra
### Requerimientos
Para realizar el presente trabajo práctico, es necesario instalar la librerías _NetCDF4_ en el sistema sobre el cual se diseñe, desarrolle y pruebe la solución al problema. Estas librerías permiten compartir información tecnológica y científica en un formato auto definido, independiente  de la arquitectura del sistema y también definen un formato de datos que se transformó en estándar abierto. La librería tiene versiones en Fortran, Python, Java y C, siendo estas últimas las que vamos a utilizar en este trabajo.
Como ejemplo del uso de este formato de datos se tiene la red de de radares meteorológicos [NexRad](https://www.ncdc.noaa.gov/data-access/radar-data/nexrad) y la constelación de satélites [GOES](https://www.ncdc.noaa.gov/data-access/satellite-data/goes-r-series-satellites), ambos disponibles públicamente.    
Si bien esta librería se encuentra en la mayoría de los repositorios de las distribuciones GNU/Linux, se recomienda, dado que buscamos optimizar los tiempos de procesamiento, se recomienda que se [compilen e instalen manualmente](https://www.unidata.ucar.edu/software/netcdf/docs/building_netcdf_fortran.html). 
También se provee de un archivo base para el uso y lectura del archivo NetCDF.

### Problema a desarrollar
Como es sabido de público conocimiento, múltiples grandes incendios han ocurrido tanto en nuestra patagonia como en bosques del Amazonas y el hemisferio honte en la última década. Estos incendios no sólo provocan perdidas materiales y de biodiversidad, sino que también múltiples pérdidas humanas. Es por esto que el monitoreo en "Tiempo Real" de estas catástrofes se ha vuelto de suma importancia. El satélite geoestacionario GOES16, posee un instrumento que se llama ABI, que obtiene 16 canales de todo el globo cada diez minutos.
Estos datos se descargan a tierra y se ponen en disponibilidad a los usuarios. La forma más común de acceder a ellos, es a travñez del _buket S3_ de acceso libre y gratuito. cada archivo tiene un formato como el siguiente ejemplo:

```bash
ABI-L2-MCMIPF/2021/066/16/OR_ABI-L2-MCMIPF-M6_G16_s20210661636116_e20210661638489_c20210661638589.nc
```

El nombre del archivo nos informa del instrumento que se utilizó el instrumento **ABI**, que es un nivel de dato **L2**, que se trata del producto **CMIPF-M6**, el satélite que lo creó **G16**, el _timestamp_ de la creación del archivo, el inicio del barrido y el final del mismo.
Una característica de este producto, es que posee todas las bandas que genera el instrumento ABI, contenido en un único archivo NetCDF.

Realizando combinaciones y operaciones sobre estos canales, se pueden generar distintos productos aplicables en distintas aplicaciones. Para el caso de un producto para detectar incendios, se utilizan los siguientes:


|          -    | **RED**      | **GREEN**      | **BLUE**     |
|---------------|:------------:|:--------------:|:------------:|
| **Name**      | Shortwave Window | Cloud Partilce Size | Snow/Ice |
| **Wavelength**| 3.9 &#181;m | 2.2 &#181;m   | 1.6 &#181;m |
| **Channel**   |      7       |       6        |      5       |
| **Units**     | Temperature (K) | Reflectance | Reflectance |
| **Range of Values**| 273.15-333.15 | 0-1 | 0-1|
| **Gamma Correction**| 0.4 |none|none|

Y se obtiene una imagen similar a la siguiente

![GOES_FIRE](https://github.com/adgko/C-Codes/blob/main/02_Memoria_Distribuida/img/index.png)

### Algoritmo ejemplo
El siguiente ejemplo desarrolllado en python describiendo paso a paso lo que ustedes tienen que hacer, pero en C. 

1 - Descargar de AWS S3 un set de datos de GOES 16, preferiblemente en horario de entre 14 y 18 UTC, utilizando la [aws_cli](https://aws.amazon.com/es/cli/)
```bash
aws s3 cp s3://noaa-goes16/ABI-L2-MCMIPF/2021/066/16/OR_ABI-L2-MCMIPF-M6_G16_s20210661636116_e20210661638489_c20210661638589.nc . --no-sign-request
``` 
Luego se abre el archivo, y se _leen_ los canales de interés.
```python
# Open the Netcdf
C = Dataset(file.nc)

# Load the three channels into appropriate R, G, and B variables
R = C['CMI_C07'][:]
G = C['CMI_C06'][:]
B = C['CMI_C05'][:]
```

La _receta_ para obtener el producto deseado

```python
# Normalizar cada canal: C = (C-minimum)/(maximum-minimum)
R = (R-273)/(333-273)
G = (G-0)/(1-0)
B = (B-0)/(0.75-0)

# Apply range limits for each channel. RGB values must be between 0 and 1
R = np.clip(R, 0, 1)
G = np.clip(G, 0, 1)
B = np.clip(B, 0, 1)

# Apply the gamma correction to Red channel.
#   corrected_value = value^(1/gamma)
gamma = 0.4
R = np.power(R, 1/gamma)
```
Y listo!

Se pide además que se guarde el resultado en un formado de imagenes (png por ejemplo).

### Restricciones
Se debe considerar que cada canal es una matriz, pero sólo hay datos dentro de la circunferencia de la tierra. No se deben realizar operaciones fuera de este.
El diseño debe contemplar toda situación no descripta en el presente documento y se debe hacer un correcto manejo de errores. 

## Desarrollo
### Programa Secuencial
Una vez entendido los puntos necesarios, se desarrolló el código secuencial que realiza los pasos que están descritos arriba en python, pero en leguaje C. Para el manejo de imagen BMP se empleó una biblioteca suministrada por la cátedra.

### Programa Paralelo
Una vez hecho funcionar el programa de manera secuencial, se proedió a modificarlo para hacerlo paralelo y emplear la biblioteca `<omp.h>`

El proceso de paralelizar el código consistió en agregar pragmas para el compilador en la sección del bucle `for` recorre la matriz de la imagen, empleando la clausula `colapse`

`#pragma omp for collapse(2)`

Para ejecutar, se programó para recibir por parámetro con cuantos hilos debía trabajar el programa.

### Problemas encontrados que hubo que tener en cuenta
La forma de recorrer la memoria en C represente un arreglo unidimensional, por lo que los saltos de fila en la matriz, se corresponden a grandes desplazamientos en columnas. Se ajustó el código de manera matricial y se dio las distintas filas a distintos hilos de ejecución.
