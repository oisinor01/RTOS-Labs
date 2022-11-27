/*


Define a structure type with 2 fields, the switch number pressed and the time of press.
Create a queue to hold 5 of the structure defined above.
The program should contain 3 tasks. There is a task for each of the 2 board switches,
SW2 and SW3.
These tasks should poll the switches every 100ms. On detection of a switch press, a local
 structure
should be filled with the switch number (2 or 3) and the time in ms when the switch was
pressed. This structure should then be queued by copy.The display task blocks waiting
for an item in the queue. On reading the queue the switch number and time should be printed.


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
static void SW2_Task(void *pvParameters);
static void SW3_Task(void *pvParameters);
static void Display_Task(void *pvParameters);
QueueHandle_t SW_Queue = NULL;
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
int main(void) {
      /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
#endif
    xTaskCreate(SW2_Task, "SW2 Task", 100, NULL, 3, NULL);
    xTaskCreate(SW3_Task, "SW3 Task", 100, NULL, 3, NULL);
    xTaskCreate(Display_Task, "Display Task", 100, NULL, 4, NULL);
    SW_Queue = xQueueCreate(5, sizeof(switchStruct));
    vQueueAddToRegistry(SW_Queue, "SW Press Queue");
    vTaskStartScheduler();
    while(1) {
    }
    return 0 ;
}
static void SW2_Task(void *pvParameters) {
    switchStruct sw2Struct;
    printf("Starting SW2 Task\n\r");
    // fullstop will help you define
    sw2Struct.switchNumber = SW2_ID;
    while(1) {
        if((GPIO_PinRead(BOARD_SW2_GPIO, BOARD_SW2_GPIO_PIN) == 0)){
            sw2Struct.switchTime = xTaskGetTickCount()*1000/configTICK_RATE_HZ;
            printf("Sending SW2 data %lu to queue\n\r", sw2Struct.switchNumber);
            xQueueSend(SW_Queue, &sw2Struct, 0);
            while(GPIO_PinRead(BOARD_SW2_GPIO, BOARD_SW2_GPIO_PIN) == 0);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
static void SW3_Task(void *pvParameters) {
    switchStruct sw3Struct;
    printf("Starting SW3 Task\n\r");
    // fullstop will help you define
    sw3Struct.switchNumber = SW3_ID;
    while(1) {
        if((GPIO_PinRead(BOARD_SW3_GPIO, BOARD_SW3_GPIO_PIN) == 0)){
            sw3Struct.switchTime = xTaskGetTickCount()*1000/configTICK_RATE_HZ;
            printf("Sending SW3 data %lu to queue\n\r", sw3Struct.switchNumber);
            xQueueSend(SW_Queue, &sw3Struct, 0);
            while(GPIO_PinRead(BOARD_SW3_GPIO, BOARD_SW3_GPIO_PIN) == 0);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
static void Display_Task(void *pvParameters){
    switchStruct displayStruct;
    printf("Starting Delay Task \n\r");
    while(1){
        // pointer to a variable so use a'&' symbol
        if(xQueueReceive(SW_Queue, &displayStruct, portMAX_DELAY) == pdTRUE){
            if(displayStruct.switchNumber == SW2_ID){
                printf("Switch 2 was pressed at this time: %lums\n\r", displayStruct.switchTime);
            }
            else if(displayStruct.switchNumber == SW3_ID){
                printf("Switch 3 was pressed at this time: %lums\n\r", displayStruct.switchTime);
            }
        }
    }
}
