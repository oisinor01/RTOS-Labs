/*




 * To demonstrate FreeRTOS software timers using the following features: -
• One-shot timer mode
• Auto-reload timer mode
Software timers are used to schedule a callback function to execute once (one-shot)
 or periodically
(auto-reload). Software timers do not use any hardware resources. CPU resources are only
used when the callback function executes

 xTimerPeriod is the timeout period in ticks.
uxAutoreload is pdFALSE for one-shot mode and pdTRUE for auto-reload mode.
pvTimerID is only used if multiple timers are using the same callback function.


A callback function needs to be created to execute when the timer expires.
There are 3 API functions to control the timer. Note: use the FromISR versions of these
API functionsif calling from an interrupt handler.xTimerReset() is equivalent to
xTimerStart() if the timer is in the dormant state. xTimerReset will restart the
timer if the timer is already running.




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

TimerHandle_t oneShotTimerHandle = NULL;
static void oneShotCallBack(TimerHandle_t xTimer);

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

    LED_RED_OFF();

    //xTaskCreate(task1, "Task 1", 100, NULL, 4, NULL);
    oneShotTimerHandle = xTimerCreate("One shot timer", pdMS_TO_TICKS(2000), pdFALSE, 0, oneShotCallBack);
    xTimerStart(oneShotTimerHandle,0);
    vTaskStartScheduler();

    while(1) {
    }
    return 0 ;
}

static void oneShotCallBack(TimerHandle_t xTimer){
	LED_RED_ON();
	printf("Timer expired at %lums \n\r", xTaskGetTickCount()*1000/configTICK_RATE_HZ);
}


/*static void task1(void *pvParameters) {
	printf("Starting Task 1\n\r");
	while(1) {
		printf("Testing\n\r");
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}
*/
