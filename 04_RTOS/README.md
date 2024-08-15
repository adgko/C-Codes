TP4 Sistemas Operativos 2
# FreeRTOS

## Iniciando
Para empezar a entender el proyecto, se procedió a compilar el demo visto en clase. El primer problema con el que se topó, se debió a que el archivo del demo que se descargaba de la página oficial estaba faltante en cuanto a un directorio. Se consiguió dicho directorio y se modificó el path en el makefile, y este fue compilado con éxito.

## Tarea Sensor
Para realizar la tarea sensor, se probó implementar la función rand() para obtener numeros aleatorios que simularan las temperaturas. No se logró que funcione, por lo que se implementó un sistema de pasar strings definidos de los números 5, 10, 15, 20, 25, 30, 35, 40, 45 y 50. Estos números van a la Queue intermedia entre esta tarea y la del filtro con xQueueSend.
Para lograr los números primero se implementó un switch case, pero este no colocaba los números, así que se pusieron todos los valores, uno detrás del otro.
ACTUALIZACION: Se entendió que los valores enteros no podían ser impresos pero si enviados sin problema, por lo que se cambió la tarea para que generara valores enteros. Durante la graficación, se vio que quedaba una curva pequeña con un máximo de 50, por lo que se cambió por 500.

## Tarea Filtro
Para la tarea filtro, se toma los valores que coloca Sensor en la Cola intermedia, y se valúan. Para cada valor, agrega a un arreglo números enteros correspondientes. Las funciones para el manejo de dicho arreglo, como así también para obtener el promedio fueron investigadas en internet y aplicadas de aquí https://www.fing.edu.uy/tecnoinf/mvd/cursos/prinprog/material/teo/prinprog-teorico07.pdf
Con ese número resultante, lo agrega a la siguiente cola para que la use luego el Graficador.
ACTUALIZACION: En un momento, dejó de funcionar la función que sacaba promedio de los valores, por lo que se agregó al main, en vez de estar modularizada como antes.

## Tarea Gráfico
La tarea Gráfico fue muy demandante y costó bastante tiempo entender. Lo que necesitaba para pasarle el dato a la función OSRAMImageDraw(), era entender que se armaba una matriz de pixeles, dividida en 2 mitades, una superior y otra inferior, y debía manipular distinto cada una. Consultando con compañeros, y mediante prueba y error, se logró un gráfico de una curva creciente, seguido de un tramo al parecer en blanco que no se pudo quitar aún.
Se evalúan los datos que toma de la Cola, se los somete a un proceso de desplazamiento y se los asigna a espacios de la matriz. Con eso se puede enviar a la función y se grafican.
Paralelo a eso, se le configuró para que imprima también el filtro N que emplea y el valor actual de la curva.

## Watermark
Para calcular el watermrk, se hizo una función printwatermark() que emplea la función uxTaskGetStackHighWaterMark() y con ello calcula cuando se ha usado del stack. Con eso podemos imprimir cuanto le queda por usar. Esta función va en el Top, imprimiendo cada vez que esta se ejecuta.
