/*
A
Create a queue to hold 5 variables of type uint32_t.
The program should contain 2 tasks. The switch task should poll SW2 every 100ms. On detection of a
switch press, the time in ms when the switch was pressed is saved to a variable. This variable should
then be queued by copy. The switch time is recorded using the xTaskGetTickCount API function.
The display task blocks waiting for an item in the queue. On reading the queue the switch number
and time should be printed.

B
Write a program to read the temperature from 10 temperature sensors every 2 seconds. Your
program should contain 2 tasks. The temperatureRead task uses the rand() function to generate 10
temperature values in the range 15 to 30 degrees. The 10 sensor readings should be copied into an
array. The array is then queued by copy. The queue should be created to hold up to 5 temperature
arrays.
The display task blocks waiting for an item in the queue. On reading the queue, 10 temperature
readings are printed.




 */

/**
 * @file    TAD_Project_Template.c
 * @brief   Application entry point.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"

/*
 * The following block is used for real time debugging statistics
 * DO NOT CHANGE THIS CODE
 *
 * To Disable real time statistics
 * 1. Disable the FTM0 peripheral using the Peripherals Config tool
 * 2. Change configGENERATE_RUN_TIME_STATS define to 0 in FreeRTOSConfig.h
 */
static uint32_t RTOS_RunTimeCounter; /* runtime counter, used for configGENERATE_RUNTIME_STATS */

void FTM0_IRQHandler(void) {

  /* Clear interrupt flag.*/

  FTM_ClearStatusFlags(FTM0, kFTM_TimeOverflowFlag);

  RTOS_RunTimeCounter++; /* increment runtime counter */
}

void RTOS_AppConfigureTimerForRuntimeStats(void) {

  RTOS_RunTimeCounter = 0;

  EnableIRQ(FTM0_IRQn);
}

uint32_t RTOS_AppGetRuntimeCounterValueFromISR(void) {

  return RTOS_RunTimeCounter;
}
/*
 * End of real time statistics code
 */

/* TODO: insert other definitions and declarations here */
static void Temperature_Task(void *pvParameters);
static void display_Task(void *pvParameters);
QueueHandle_t temperature_Queue = NULL;

/*
 * @brief   Application entry point.
 */
int main(void) {

  	/* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
#endif

    xTaskCreate(Temperature_Task, "Temperature task", 100, NULL, 3, NULL);
    xTaskCreate(display_Task, "Display task", 100, NULL, 4, NULL);
    temperature_Queue = xQueueCreate(5, 10*sizeof(uint8_t));
    vQueueAddToRegistry(temperature_Queue, "sw2 press queue");
    vTaskStartScheduler();

    while(1) {
    }
    return 0 ;
}

static void Temperature_Task(void *pvParameters) {
	uint32_t tempValues[10],x ;
	printf("Starting Temperature 1\n\r");
	while(1){
	//genreate random numbers from 15 to 30 and add to each array
		//send array to queue and repeat every 5 seconds
		xQueueSend(temperature_Queue, tempValues,0);
		vTaskDelay(pdMS_TO_TICKS(5000));

		for( x =0; x< 10; x++){
			tempValues[x]= rand()% 16 + 15;
			printf("Temperature : %lu\n\r", tempValues[x] );
		}
		xQueueSend(temperature_Queue,tempValues,0);
		vTaskDelay(pdMS_TO_TICKS(500));

	}
}

static void display_Task(void *pvParameters)
{
	uint8_t sensorValues[10],x;
	printf("Starting display time");
	while(1){
		if(xQueueReceive(temperature_Queue, sensorValues, portMAX_DELAY)== pdTRUE){
			for( x=0; x>10; x++){
			printf("Temperature : %lu\n\r", sensorValues[x] );
			}
		}
	}
}
