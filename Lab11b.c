/*
 *
 * In auto-reload mode, the callback function runs when the timer expires, and the timer
 *  then restarts and continues in the running state. Create an application to toggle
 *  the red LED every 2 seconds. A software timer is created in autoreload mode with a
 *  timeout period of 2 seconds. The timer and scheduler are then started. The software
 *   timer callback function will increment a local variable called sec and print the new
 *   time.
 *
 */

/**
 * @file    TAD_Project_Template.c
 * @brief   Application entry point.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

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
//static void task1(void *pvParameters);

static void task1 ( void *pvParameters);
TimerHandle_t autoReloadTimerHandle = NULL;
static void autoReloadCallBack(TimerHandle_t xTimer);
TaskHandle_t task1Handle = NULL;

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

    LED_RED_TOGGLE();

    //xTaskCreate(task1, "Task 1", 100, NULL, 4, NULL);
    autoReloadTimerHandle = xTimerCreate("auto shot timer", pdMS_TO_TICKS(2000), pdTRUE, 0, autoReloadCallBack);
    xTimerStart(autoReloadCallBack,0);
    vTaskStartScheduler();

    while(1) {
    }
    return 0 ;
}

static void autoReloadCallBack(TimerHandle_t xTimer){
	LED_RED_ON();
	printf("Timer expired at %lums \n\r", xTaskGetTickCount()*1000/configTICK_RATE_HZ);
}


//this code shows us how a software timer can sync tasks
static void task1(void *pvParameters) {
	uint32_t TN_Value;
	printf("Starting Task 1\n\r");
	while(1) {
		TN_Value = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(TN_Value != 0){
			LED_RED_TOGGLE();
			printf("Task notification received \r\n");
		}
	}
}

