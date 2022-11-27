/**Project description**
 *
 *
 * This application will simulate a display backlight using the red LED.
 * The backlight will turn on for 5 seconds when a key is pressed on the keyboard.
 * The backlight timeout is extended by another 5 seconds if another key is pressed
 * before the expiration of the timeout.
 *
 * Create an event group with 2 bits to turn the backlight on and off.
 * The ON bit is used to synchronise the UART interrupt handler with the backlight task.
 * The OFF bit will synchronise the timer callback function with the backlight task.
 * The UART handler will restart the timer and set the event group ON bit. The software
 * timer callback function will set the event group OFF bit.The backlight task will wait
 * for either event group bit to be set before changing the backlight LED.
 *
 *
* @file    TAD_Project_Template.c
* @brief   Application entry point.
*/
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "event_groups.h"
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
static void backLightTask(void *pvParameters);
TimerHandle_t oneShotTimerHandle = NULL;
//standard function protorype.
static void oneShotCallBack(TimerHandle_t xTimer);
TaskHandle_t BackLightHandle = NULL;


/* UART0_RX_TX_IRQn interrupt handler */
void UART0_SERIAL_RX_TX_IRQHANDLER(void) {
    BaseType_t xHigherPriorityTaskWoken;
    BaseType_t ch;
  uint32_t intStatus;
  /* Reading all interrupt flags of status registers */
  intStatus = UART_GetStatusFlags(UART0_PERIPHERAL);
  /* Flags can be cleared by reading the status register and reading/writing data registers.
    See the reference manual for details of each flag.
    The UART_ClearStatusFlags() function can be also used for clearing of flags in case the content of data regsiter is not used.
    For example:
        status_t status;
        intStatus &= ~(kUART_RxOverrunFlag | kUART_NoiseErrorFlag | kUART_FramingErrorFlag | kUART_ParityErrorFlag);
        status = UART_ClearStatusFlags(UART0_PERIPHERAL, intStatus);
  */
  /* Place your code here */
  ch = UART_ReadByte(UART0);
  xHigherPriorityTaskWoken = pdFALSE;
  //sent notification value of 1
  // reet timer
  //xTaskNotifyFromISR(TimerTaskHandle, totalSeconds, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
  xTaskNotifyFromISR(BackLightHandle, 1, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
  xTimerResetFromISR(oneShotTimerHandle, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

  /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F
     Store immediate overlapping exception return operation might vector to incorrect interrupt. */
  #if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
  #endif
}


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
    xTaskCreate(backLightTask, "Back Light Task", 100, NULL, 3, &BackLightHandle);
    oneShotTimerHandle = xTimerCreate("one shot timer", pdMS_TO_TICKS(5000), pdTRUE, 0, oneShotCallBack);
    xTimerStart(oneShotTimerHandle, 0);
    vTaskStartScheduler();
    while(1) {
    }
    return 0 ;
}
static void oneShotCallBack(TimerHandle_t xTimer){
    //send task notification with value of 2
    xTaskNotify(BackLightHandle, 2, eSetValueWithOverwrite);

    // gives us the number of ticks since we started the scheduler.
//    printf("Timer expired at: %lums", xTaskGetTickCount()*1000/configTICK_RATE_HZ);

}

static void backLightTask(void *pvParameters) {
    uint32_t TN_Value;
    printf("Starting Task 1\n\r");
    while(1) {
        TN_Value = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        //2 shources that snd a task notification
        if(TN_Value == 1){
            //TN froom UART
            printf("Key pressed at %lum, BackLight on\n\r", xTaskGetTickCount()*1000/configTICK_RATE_HZ);
            LED_RED_ON();
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
        else if (TN_Value == 2){
            // TN from task
            printf("Timer expired at: %lums, Backlight off\n\r", xTaskGetTickCount()*1000/configTICK_RATE_HZ);
            LED_RED_OFF();
        }

    }
}
