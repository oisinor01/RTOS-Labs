#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_debug_console.h"
/* TODO: insert other include files here. */
/* TODO: insert other definitions and declarations here. *
*/
static void TimerTask(void *pvParameters);
SemaphoreHandle_t PIT_Semaphore = NULL;
TaskHandle_t timerTaskHandle=NULL;
/*
* @brief   Application entry point.
*/
/* PIT0_IRQn interrupt handler */
void PIT_CHANNEL_0_IRQHANDLER(void) {
    BaseType_t xHigherPriorityTaskWoken;
    uint32_t intStatus;
    /* Reading all interrupt flags of status register */
    intStatus = PIT_GetStatusFlags(PIT_PERIPHERAL, PIT_CHANNEL_0);
    PIT_ClearStatusFlags(PIT_PERIPHERAL, PIT_CHANNEL_0, intStatus);
    /* Place your code here */
    xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(PIT_Semaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F
     Store immediate overlapping exception return operation might vector to incorrect interrupt. */
#if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
#endif
}
/* PORTC_IRQn interrupt handler */
/* UART0_RX_TX_IRQn interrupt handler */
void UART0_SERIAL_RX_TX_IRQHANDLER(void) {
    static uint8_t buffer[10] = "0", index = 0;
    uint8_t hour,min,sec;
    uint32_t totalSeconds;
    BaseType_t xHigherPriorityTaskWoken;
    uint32_t intStatus;
    /* Reading all interrupt flags of status registers */
    intStatus = UART_GetStatusFlags(UART0_PERIPHERAL);
    if (intStatus & kUART_RxDataRegFullFlag) {
        buffer[index] = UART_ReadByte(UART0);
        if(buffer[index] == '\r'){    //'/r' is a character return
            //parse string
            hour = (buffer[0] - '0')*10 + (buffer[1] - '0'); // *10 e.g 1 and 2 is entered = 3, 1*10 and 2 is entered = 12
            min =  (buffer[3] - '0')*10 + (buffer[4] - '0');
            sec =  (buffer[6] - '0')*10 + (buffer[7] - '0');
            if(hour<24 && min<=59 && sec<=59){
            totalSeconds = hour*3600 + min*60 + sec;
            xHigherPriorityTaskWoken = pdFALSE;
            xTaskNotifyFromISR(timerTaskHandle,totalSeconds,eSetValueWithOverwrite,&xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
            else{
                printf("Invalid\n\r");
            }
            index =0;
            memset(buffer,0,sizeof(buffer));
        }
        else{
            index++;
        }
        }

    /* Flags can be cleared by reading the status register and reading/writing data registers.
     See the reference manual for details of each flag.
     The UART_ClearStatusFlags() function can be also used for clearing of flags in case the content of data regsiter is not used.
     For example:
     status_t status;
     intStatus &= ~(kUART_RxOverrunFlag | kUART_NoiseErrorFlag | kUART_FramingErrorFlag | kUART_ParityErrorFlag);
     status = UART_ClearStatusFlags(UART0_PERIPHERAL, intStatus);
     */
    /* Place your code here */
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
    xTaskCreate(TimerTask, "TimerTask", 200, NULL, 4, &timerTaskHandle);
    PIT_Semaphore = xSemaphoreCreateBinary();
    vTaskStartScheduler();
    while (1) {
    }
    return 0;
}
void TimerTask(void *pvParameters) {
    uint32_t count = 0;
    uint8_t Flag = 1;
    //ruint8_t UART_Bits;
    uint32_t notificationValue;
    printf("Starting Timer\n\r");
    printf("Timer: %02d:%02d:%02d\r\n", count/3600,(count%3600)/60,count%60);
    while (1) {
        if (xSemaphoreTake(PIT_Semaphore, 1) == pdTRUE) {
            if (++count == 86400) {
            count = 0;
            }
            printf("Timer: %02d:%02d:%02d\r\n", count/3600,(count%3600)/60,count%60);

        }
        notificationValue = ulTaskNotifyTake(pdTRUE,1);
        if (notificationValue != 0){  //notification
            count= notificationValue;
            printf("***Time Changed***\r\nTimer: %02d:%02d:%02d\r\n", count/3600,(count%3600)/60,count%60);
        }
    }
}
