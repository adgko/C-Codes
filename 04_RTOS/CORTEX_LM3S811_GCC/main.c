/*
 * FreeRTOS V202112.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://aws.amazon.com/freertos
 *
 */

/*
 * This project contains an application demonstrating the use of the
 * FreeRTOS.org mini real time scheduler on the Luminary Micro LM3S811 Eval
 * board.  See http://www.FreeRTOS.org for more information.
 *
 * main() simply sets up the hardware, creates all the demo application tasks,
 * then starts the scheduler.  http://www.freertos.org/a00102.html provides
 * more information on the standard demo tasks.
 *
 * In addition to a subset of the standard demo application tasks, main.c also
 * defines the following tasks:
 *
 * + A 'Print' task.  The print task is the only task permitted to access the
 * LCD - thus ensuring mutual exclusion and consistent access to the resource.
 * Other tasks do not access the LCD directly, but instead send the text they
 * wish to display to the print task.  The print task spends most of its time
 * blocked - only waking when a message is queued for display.
 *
 * + A 'Button handler' task.  The eval board contains a user push button that
 * is configured to generate interrupts.  The interrupt handler uses a
 * semaphore to wake the button handler task - demonstrating how the priority
 * mechanism can be used to defer interrupt processing to the task level.  The
 * button handler task sends a message both to the LCD (via the print task) and
 * the UART where it can be viewed using a dumb terminal (via the UART to USB
 * converter on the eval board).  NOTES:  The dumb terminal must be closed in
 * order to reflash the microcontroller.  A very basic interrupt driven UART
 * driver is used that does not use the FIFO.  19200 baud is used.
 *
 * + A 'check' task.  The check task only executes every five seconds but has a
 * high priority so is guaranteed to get processor time.  Its function is to
 * check that all the other tasks are still operational and that no errors have
 * been detected at any time.  If no errors have every been detected 'PASS' is
 * written to the display (via the print task) - if an error has ever been
 * detected the message is changed to 'FAIL'.  The position of the message is
 * changed for each write.
 */

/* Environment includes. */
#include "DriverLib.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "stdio.h"			// sprintf()
#include <string.h>			//sino, no saca strlen()
#include "osram96x16.h"		// para el display
#include "rand.h"			


/* Delay between cycles of the 'check' task. */
#define mainCHECK_DELAY ((TickType_t)5000 / portTICK_PERIOD_MS)
#define mainTOP_DELAY ((TickType_t)6000 / portTICK_PERIOD_MS) //distinto tiempo porque sino se cruzan los mensajes

/* UART configuration - note this does not use the FIFO so is not very
efficient. */
#define mainBAUD_RATE (19200)
#define mainFIFO_SET (0x10)

/* Misc. */
#define mainQUEUE_SIZE (20)
#define mainDEBOUNCE_DELAY ((TickType_t)150 / portTICK_PERIOD_MS)
#define mainNO_DELAY ((TickType_t)0)

unsigned int N;		// si lo defino con #define no se puede cambiar
unsigned int N_aux; // el que paso por interrupción de UART



// función para imprimir por UART los mensajaes y la task TOP
//static void uartPrint(char * msg,uint8_t len);

/* String that is transmitted on the UART. */
// static char *cMessage = "Task woken by button interrupt! --- ";
static volatile char *pcNextChar;

/* The queue used to send strings to the print task for display on the LCD. */
QueueHandle_t xPrintQueue;
QueueHandle_t xUARTQueue;

/* guarda el "id" de la tarea al momento de crearla, es necesario para la implementacion del watermark*/
TaskHandle_t xHandle_Sensor = NULL;
TaskHandle_t xHandle_Filter = NULL;
TaskHandle_t xHandle_Grafic = NULL;
TaskHandle_t xHandle_Top = NULL;

/*-----------------------------------------------------------*/


void vTempSensor(void *pvParameters);
QueueHandle_t xSensorQueue;
void vFilter(void *pvParameters);
uint8_t getAverage(uint8_t *lastMeasurements, uint8_t totalMeasurements);
uint8_t delayTime = pdMS_TO_TICKS(100);
static void uartPrint(char * msg,uint8_t len);
static void vTop(void *pvParameters);
static void vprintWaterMark(void *pvParameters);
void getTasksStats(char *writeUART);
void printStatistics(char *taskName, UBaseType_t waterMark);

char *longToChar(unsigned long value, char *ptr, int base);
// funciones usadas
// función para pasar de int a char
char *itoa(int val);

/*
 * Configure the processor and peripherals for this demo.
 */
static void prvSetupHardware(void);

/*
 * The task that controls access to the LCD.
 */
static void vPrintTask(void *pvParameter);

// función para imprimir el stack por UART
static void printWaterMark( );

int main(void)
{
	/* Configure the clocks, UART and GPIO. */
	prvSetupHardware();

	/* Create the queue used to pass message to vPrintTask. */
	xPrintQueue = xQueueCreate(mainQUEUE_SIZE, sizeof(uint8_t));
	xSensorQueue = xQueueCreate(mainQUEUE_SIZE, sizeof(char *));
	xUARTQueue = xQueueCreate(mainQUEUE_SIZE, sizeof(char *));

	xTaskCreate(vPrintTask, "Print", configMINIMAL_STACK_SIZE, NULL, 0, &xHandle_Grafic);
	xTaskCreate(vTempSensor, "Sensor", configMINIMAL_STACK_SIZE, NULL, 0, &xHandle_Sensor);
	xTaskCreate(vFilter, "Filter", configMINIMAL_STACK_SIZE, NULL, 0, &xHandle_Filter);
	xTaskCreate(vTop, "Top", configMINIMAL_STACK_SIZE, NULL, 0, &xHandle_Top);
	xTaskCreate(vprintWaterMark, "uartPrint", configMINIMAL_STACK_SIZE, NULL, 0, &xHandle_Top);

	/* Start the scheduler. */
	vTaskStartScheduler();

	/* Will only get here if there was insufficient heap to start the
	scheduler. */

	return 0;
}

static void prvSetupHardware(void)
{
	// Setup the PLL.
	SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_6MHZ);

	// Enable the UART.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

	// Configure the UART for 8-N-1 operation.
	UARTConfigSet(UART0_BASE, mainBAUD_RATE, UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE);

	// Enable Tx interrupts.
	UARTIntEnable(UART0_BASE, UART_INT_RX);
	UARTIntEnable(UART0_BASE, UART_INT_TX);
	IntPrioritySet(INT_UART0, configKERNEL_INTERRUPT_PRIORITY);
	IntEnable(INT_UART0);

	// Initialise the LCD
	OSRAMInit(false);
	OSRAMStringDraw("www.FreeRTOS.org", 0, 0);
	OSRAMStringDraw("LM3S811 demo", 16, 1);
}

void vUART_ISR(void)
{
	unsigned long ulStatus;
	char *rxChar;
	unsigned long retval;
	unsigned int n = 10;
	int rxNum;

	// What caused the interrupt.
	ulStatus = UARTIntStatus(UART0_BASE, pdTRUE);

	// Clear the interrupt.
	UARTIntClear(UART0_BASE, ulStatus);

	// Was a Tx interrupt pending?
	if (ulStatus & UART_INT_TX)
	{
		// Send the next character in the string.  We are not using the FIFO.
		if (*pcNextChar != 0)
		{

			if (!(HWREG(UART0_BASE + UART_O_FR) & UART_FR_TXFF))
			{

				HWREG(UART0_BASE + UART_O_DR) = *pcNextChar;
			}
			pcNextChar++;
		}
	}

	//Recibe números en ASCII, por lo que tiene que evaluar
	// 0 = 48
	// 1 = 49
	// 2 = 50
	// 3 = 51
	// 4 = 52
	// 5 = 53
	// 6 = 54
	// 7 = 55
	// 8 = 56
	// 9 = 57
	// cuando un valor por UART, revisa que sea un número,
	// luego le resta 48 y obtiene el número de filtro
	if (ulStatus & UART_INT_RX)
	{

		// rxNum = UARTCharGet(UART0_BASE) - 48;
		retval = UARTCharGet(UART0_BASE);
		if (retval >= 48 & retval <= 57)
		{ // recibo en ASCII
			retval -= 48;
			if (retval == 0)
			{
				n = 10;
			}
			else
			{
				n = retval;
			}
			xQueueSend(xUARTQueue, &n, portMAX_DELAY);
		}

	}
}

/*-----------------------------------------------------------------------*/
//Grafica los valores de temperatura en el tiempo, en el LCD se muestra el valor de N actual y el promedio de las mediciones
//hay que pasarle a la función OSRAMStringDraw los valores en char o no los toma
void vPrintTask(void *pvParameters)
{
	uint8_t pcMessage;
	uint8_t imageGraph[192] = {0};

	char *valor_filtro = "10";
	char *valor_grafico;

	OSRAMClear();

	// Linea del eje Y (2^8=255)
	imageGraph[0] = 255;
	imageGraph[96] = 255;

	for (;;)
	{

		// Wait for a message to arrive.
		xQueueReceive(xPrintQueue, &pcMessage, portMAX_DELAY);

		valor_grafico = itoa(pcMessage); // convierte el valor a char
		valor_filtro = itoa(N);			 // porque sprintf/() no funca

		for (uint8_t i = 191; i > 0; i--)
		{
			if (i == 1 || i == 97 || i == 96)
			{
				continue;
			}
			imageGraph[i] = imageGraph[i - 1];
		}
		// imageGraph[97] = 0b10000000; //Linea del eje X

		switch (pcMessage)
		{
		case 30:
			imageGraph[97] = 0b10000100;
			break;
		case 31:
			imageGraph[97] = 0b10000010;
			break;
		case 32:
			imageGraph[97] = 0b10000001;
			break;
		case 33:
			imageGraph[1] = 0b10000000;
			imageGraph[97] = 0b10000000; // Linea del eje X
			break;
		case 34:
			imageGraph[1] = 0b01000000;
			imageGraph[97] = 0b10000000; // Linea del eje X
			break;
		case 35:
			imageGraph[1] = 0b00100000;
			imageGraph[97] = 0b10000000; // Linea del eje X
			break;
		case 36:
			imageGraph[1] = 0b00010000;
			imageGraph[97] = 0b10000000; // Linea del eje X
			break;
		case 37:
			imageGraph[1] = 0b00001000;
			imageGraph[97] = 0b10000000; // Linea del eje X
			break;
		case 38:
			imageGraph[1] = 0b00000100;
			imageGraph[97] = 0b10000000; // Linea del eje X
			break;
		case 39:
			imageGraph[1] = 0b00000010;
			imageGraph[97] = 0b10000000; // Linea del eje X
			break;
		default:
			imageGraph[1] = imageGraph[2];
			imageGraph[97] = 0b10000000; // Linea del eje X
			break;
		}

		// Tomo 96 coñumnas y 2 filas, en eso dibujo desde
		//  el eje 15, antes pongo el número de filtro y
		//  el valor de temperatura que tengo
		OSRAMStringDraw(valor_filtro, 0, 0);
		OSRAMStringDraw(valor_grafico, 0, 1);
		OSRAMImageDraw(imageGraph, 15, 0, 96, 2);
	}
}

//Toma un valor aleatorio y lo divido en modulo por 11 par tener 
// un valor entre 0 y 9
// a eso le sumo 30 y obtengo valores entre 30 y 39
void vTempSensor(void *pvParameters)
{
	uint8_t temperature = 0;

	for (;;)
	{
		temperature = (uint8_t)(rand() % 11) + 30; // Valores de aprox 30 grados

		xQueueSend(xSensorQueue, &temperature, portMAX_DELAY);

		vTaskDelay(delayTime);
	}
}
/*-----------------------------------------------------------------------*/
//Filtro pasabajo, se calcula un pormedio de las ultimas N mediciones del sensor. N es ingresado por UART, inicializado en 1
void vFilter(void *pvParameters)
{
	uint8_t *lastMeasurements = NULL;
	uint8_t position = -1;
	uint8_t totalMeasurements = 10;
	uint8_t auxTemp = 0;
	uint8_t average = 0;
	lastMeasurements = (uint8_t *)pvPortMalloc(sizeof(uint8_t) * totalMeasurements);

	for (;;)
	{
		xQueueReceive(xSensorQueue, &auxTemp, portMAX_DELAY);
		if (xQueueReceiveFromISR(xUARTQueue, &N_aux, 0) == pdTRUE)
		{
			if (N_aux > 0)
			{
				position = totalMeasurements; // valor necesario para ver la posición del areglo donde está
				totalMeasurements = N_aux;	  // el total de mediciones que tomo, seleccionadas por UART
				N = N_aux;					  // las medidas tomasdas por UART pasadas a N, para el gráfico
				lastMeasurements = (uint8_t *)pvPortMalloc(sizeof(uint8_t) * totalMeasurements);
			}
		}
		if (auxTemp > 39)
		{
			auxTemp = (uint8_t)(rand() % 10) + 30;
		}
		lastMeasurements[(position++) % totalMeasurements] = auxTemp;		//si baja el N, se sigue llenando de a 10 (esto es un error)
		average = getAverage(lastMeasurements, totalMeasurements);
		xQueueSend(xPrintQueue, &average, portMAX_DELAY);
		vTaskDelay(delayTime);
	}
}

static void vTop(void *pvParameters)
{

	TickType_t xLastExecutionTime;
	char writeUART[150];

	xLastExecutionTime = xTaskGetTickCount();

	for (;;)
	{
		/* Perform this check every mainCHECK_DELAY milliseconds. */
		vTaskDelayUntil(&xLastExecutionTime, mainTOP_DELAY);

		getTasksStats(writeUART);

		UARTIntDisable(UART0_BASE, UART_INT_TX);
		{
			pcNextChar = writeUART;

			/* Send the first character. */
			if (!(HWREG(UART0_BASE + UART_O_FR) & UART_FR_TXFF))
			{
				HWREG(UART0_BASE + UART_O_DR) = *pcNextChar;
			}

			pcNextChar++;
		}
		UARTIntEnable(UART0_BASE, UART_INT_TX);

	}
}

/* ######################################################################## */
/* ################################# FUNCIONES ############################ */
/* ######################################################################## */

//Hace sumatoria de todos los valores y lo divide por la cantidad N_aux
uint8_t getAverage(uint8_t *lastMeasurements, uint8_t totalMeasurements)
{
	uint16_t aux = 0;

	for (uint8_t i = 0; i < totalMeasurements; i++)
	{
		aux += lastMeasurements[i];
	}
	return aux / totalMeasurements;
}

//Convertidor de long a char para imprimir
char *longToChar(unsigned long value, char *ptr, int base)
{

	unsigned long t = 0, res = 0;
	unsigned long tmp = value;
	int count = 0;

	if (NULL == ptr)
		return NULL;

	if (tmp == 0)
		count++;

	while (tmp > 0)
	{
		tmp = tmp / base;
		count++;
	}

	ptr += count;

	*ptr = '\0';

	do
	{

		res = value - base * (t = value / base);
		if (res < 10)
			*--ptr = '0' + res;

		else if ((res >= 10) && (res < 16))
			*--ptr = 'A' - 10 + res;

	} while ((value = t) != 0);

	return (ptr);
}


//Rediseñado. Anteriormente basé el código de vTaskList() en Source/task.c
// Ese código usaba sprintf para ir almacenando los valores en un buffer
// lo reemplacé por strcat y ahora funciona
void getTasksStats(char *writeUART)
{

	TaskStatus_t *pxTaskStatusArray;
	volatile UBaseType_t uxArraySize, x;
	uint32_t ulTotalRunTime, ulStatsAsPercentage;
	char buffer[10];

	*writeUART = 0x00;

	uxArraySize = uxTaskGetNumberOfTasks();

	pxTaskStatusArray = pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

	if (pxTaskStatusArray != NULL)
	{
		// Generate raw status information about each task.
		uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime);

		// For percentage calculations.
		ulTotalRunTime /= 100UL;

		// Avoid divide by zero errors.
		if (ulTotalRunTime > 0)
		{

			strcat(writeUART, "Task\t\tRuntime\t\tRuntime [%]\n");
			strcat(writeUART, "----------------------------------------------\n");

			for (x = 0; x < uxArraySize; x++)
			{
				// What percentage of the total run time has the task used?
				// This will always be rounded down to the nearest integer.
				// ulTotalRunTimeDiv100 has already been divided by 100.
				ulStatsAsPercentage = pxTaskStatusArray[x].ulRunTimeCounter / ulTotalRunTime;

				if (ulStatsAsPercentage > 0UL)
				{
					strcat(writeUART, pxTaskStatusArray[x].pcTaskName);
					strcat(writeUART, "\t\t");
					strcat(writeUART, longToChar(pxTaskStatusArray[x].ulRunTimeCounter, buffer, 10));
					strcat(writeUART, "\t\t");
					strcat(writeUART, longToChar(ulStatsAsPercentage, buffer, 10));
					strcat(writeUART, "%\r\n");
				}
				else
				{
					// If the percentage is zero here then the task has
					// consumed less than 1% of the total run time.
					strcat(writeUART, pxTaskStatusArray[x].pcTaskName);
					strcat(writeUART, "\t\t");
					strcat(writeUART, longToChar(pxTaskStatusArray[x].ulRunTimeCounter, buffer, 10));
					strcat(writeUART, "\t\t");
					strcat(writeUART, "1%");
					strcat(writeUART, "\r\n");
				}

				writeUART += strlen((char *)writeUART);
			}
			strcat(writeUART, "\n\n");
			writeUART += strlen((char *)writeUART);
		}
		// The array is no longer needed, free the memory it consumes.
		vPortFree(pxTaskStatusArray);
	}
}

// función que pasa de int a char
//  toma el valor de int, busca en ascii su equivalente
//  y lo almacena en un arreglo que devuelve
//  función tomada de https://stackoverflow.com/questions/18061609/itoa-function-for-gcc
char *itoa(int val)
{
	static char buf[32] = {0};
	int i = 30;
	for (; val && i; --i, val /= 10)
		buf[i] = "0123456789abcdef"[val % 10];
	return &buf[i + 1];
}

// Función que calcula el watermark de cada tarea
// Revisa cuanto usó de la pila
// LE resta al total de 280 bytes la cantidad de bytes usados por la tarea
// Y lo imprime
//
// ACTUALIZACION: tuve que convertirla en una task porque como funcion en el top
// me rompía la gráfica y  no podía ver los resultados
static void vprintWaterMark(void *pvParameters ){

	char  watermark[50];
	// para guardar todos los stacks
	int STACK_vSensorTask;
	int STACK_vFiltroTask;
	int STACK_vGraficTask;
	int STACK_vTopTask;
	int STACK;
	char *mensaje_sensor = "Sensor Task: ";
	char *mensaje_filtro = "Filtro Task: ";
	char *mensaje_grafic = "Grafic Task: ";
	char *mensaje_stack = "Stack: ";
	char *mensaje_top = "Top Task: ";
	char mensaje_watermark[50] = "========= WATERMARKS =========";

	UBaseType_t uxHighWaterMark_Sensor;
	UBaseType_t uxHighWaterMark_Filter;
	UBaseType_t uxHighWaterMark_Grafic;
	UBaseType_t uxHighWaterMark_UART;
	UBaseType_t uxHighWaterMark_Stack;
	UBaseType_t uxHighWaterMark_Top;

	for(;;){
	uartPrint(mensaje_watermark,strlen(mensaje_watermark));

	memset(watermark, 0 , sizeof(char)*50 );
	uxHighWaterMark_Sensor = uxTaskGetStackHighWaterMark(xHandle_Sensor);
	STACK_vSensorTask = (configMINIMAL_STACK_SIZE - (uxHighWaterMark_Sensor));
	strcat(watermark, mensaje_sensor);
	strcat(watermark, itoa(STACK_vSensorTask));
	uartPrint(watermark,strlen(watermark));

	memset(watermark, 0 , sizeof(char)*50 );
	uxHighWaterMark_Filter = uxTaskGetStackHighWaterMark(xHandle_Filter);
	STACK_vFiltroTask = (configMINIMAL_STACK_SIZE - (uxHighWaterMark_Filter));
	strcat(watermark, mensaje_filtro);
	strcat(watermark, itoa(STACK_vFiltroTask));
	uartPrint(watermark,strlen(watermark));

	memset(watermark, 0 , sizeof(char)*50 );
	uxHighWaterMark_Grafic = uxTaskGetStackHighWaterMark(xHandle_Grafic);
	STACK_vGraficTask = (configMINIMAL_STACK_SIZE - (uxHighWaterMark_Grafic));
	strcat(watermark, mensaje_grafic);
	strcat(watermark, itoa(STACK_vGraficTask));
	uartPrint(watermark,strlen(watermark));
	
	memset(watermark, 0 , sizeof(char)*50 );
	uxHighWaterMark_Top = uxTaskGetStackHighWaterMark(xHandle_Top);
	STACK_vTopTask = (configMINIMAL_STACK_SIZE - (uxHighWaterMark_Top));
	strcat(watermark, mensaje_top);
	strcat(watermark, itoa(STACK_vTopTask));
	uartPrint(watermark,strlen(watermark));

	memset(watermark, 0 , sizeof(char)*50 );
	uxHighWaterMark_Stack = uxTaskGetStackHighWaterMark(NULL); //devuelve el espacio que quedo vacio en la pila
	STACK = (configMINIMAL_STACK_SIZE - (uxHighWaterMark_Stack));
	strcat(watermark, mensaje_stack);
	strcat(watermark, itoa(STACK));
	uartPrint(watermark,strlen(watermark));

	vTaskDelay(delayTime*100);
	}
}

// envia el mensaje indicado caracter por caracter por el puerto serie 
static void uartPrint(char * msg,uint8_t len)
{
	//recorro el msg caracter por caracter y lo transmito
	for(uint8_t i=0;i<len;i++){
		//UARTCharPut(UART0_BASE,(unsigned)msg[i]);
		UARTCharPut(UART0_BASE,msg[i]);
	}
	//imprimo un return carriage al final
	UARTCharPut(UART0_BASE,'\n');
}