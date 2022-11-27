/*


To demonstrate FreeRTOS queues using the following features: -
• Queue by copy
• Queue by reference
• UART interfacing interrupts
A queue is the primary method to communicate data in a thread safe manner between 2 tasks or
between an interrupt handler and a task.




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
static void Display_Task(void *pvParameters);
QueueHandle_t UART_Queue = NULL;
uint8_t buffer[40] = "0", index = 0;
BaseType_t xHigherPriorityTaskWoken;
// structure is like an array but you can have different variables instead of just one
typedef struct{
    uint8_t switchNumber;
    uint32_t switchTime;
}switchStruct;
#define SW2_ID 2
#define SW3_ID 3
/*
* @brief   Application entry point.
*/
/* UART0_RX_TX_IRQn interrupt handler */
void UART0_SERIAL_RX_TX_IRQHANDLER(void) {
  uint32_t intStatus;
  uint8_t *rxPtr;
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
  buffer[index] = UART_ReadByte(UART0);
  if(buffer[index] == '\r'){
      printf("String received: %s", buffer);
      printf("Sending string pointer to queue\r\n");
      rxPtr = buffer;
      xHigherPriorityTaskWoken = pdFALSE;
      xQueueSendFromISR(UART_Queue, &rxPtr, &xHigherPriorityTaskWoken);
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
  else{
      index++;
  }
  /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F
     Store immediate overlapping exception return operation might vector to incorrect interrupt. */
  #if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
  #endif
}

int main(void) {
      /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
#endif

    xTaskCreate(Display_Task, "Display Task", 100, NULL, 4, NULL);
    UART_Queue = xQueueCreate(5, sizeof(uint8_t *));
    vQueueAddToRegistry(UART_Queue, "UART Queue");
    vTaskStartScheduler();
    while(1) {
    }
    return 0 ;
}


static void Display_Task(void *pvParameters){
    uint8_t *displayPtr;
    char *ptr;
    char timeBuff[10]="0", tempBuff[10]="0",humBuff[10] ="0", i;
    printf("Starting Delay Task \n\r");
    while(1){
        // pointer to a variable so use a'&' symbol
        if(xQueueReceive(UART_Queue, &displayPtr, portMAX_DELAY) == pdTRUE){
            printf("String from queue: %s\n\r", displayPtr);
            ptr = strstr((char *)displayPtr, "T-");
            if(ptr){
            	ptr = strchr(ptr, '-');
            	ptr++;
            	i=0;
            	memset(timeBuff,0,sizeof(timeBuff));
            	while(*ptr != ';'){
            		timeBuff[i++] = *ptr++;

            	}
            	printf("Temperature: %s ", timeBuff);
            }
            //parse data
            index=0;
            memset(buffer,0,sizeof(buffer));
        }
    }
}
