/*
* Copyright 2016-2022 NXP
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* o Redistributions of source code must retain the above copyright notice, this list
*   of conditions and the following disclaimer.
*
* o Redistributions in binary form must reproduce the above copyright notice, this
*   list of conditions and the following disclaimer in the documentation and/or
*   other materials provided with the distribution.
*
* o Neither the name of NXP Semiconductor, Inc. nor the names of its
*   contributors may be used to endorse or promote products derived from this
*   software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**
* @file    Lab7.c
* @brief   Application entry point.
*/
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
TaskHandle_t TimerTaskHandle = NULL;
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
/* UART0_RX_TX_IRQn interrupt handler */
void UART0_SERIAL_RX_TX_IRQHANDLER(void) {
    uint8_t ch;
    BaseType_t xHigherPriorityTaskWoken;
    uint32_t intStatus;
    /* Reading all interrupt flags of status registers */
    intStatus = UART_GetStatusFlags(UART0_PERIPHERAL);
    if (intStatus & kUART_RxDataRegFullFlag) {
        ch = UART_ReadByte(UART0);
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
    xHigherPriorityTaskWoken = pdFALSE;
    // set event bit based on character received
    switch(ch) {
    case 'r': vTaskNotifyGiveFromISR(TimerTaskHandle, &xHigherPriorityTaskWoken); break;
    default: break;
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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
    xTaskCreate(TimerTask, "TimerTask", 200, NULL, 4, &TimerTaskHandle);
    PIT_Semaphore = xSemaphoreCreateBinary();
    vTaskStartScheduler();
    while (1) {
    }
    return 0;
}
void TimerTask(void *pvParameters) {
    uint8_t count = 0;
    uint32_t notificationValue;
    printf("Starting Timer\n\r");
    printf("Timer: %02d:%02d:%02d\r\n", count/3600, count%3600/60, count%60);
    while (1) {
        if (xSemaphoreTake(PIT_Semaphore, 1) == pdTRUE) {
            count++;
            printf("Timer: %02d:%02d:%02d\r\n", count/3600, count%3600/60, count%60);
            }
        notificationValue = ulTaskNotifyTake(pdTRUE, 1);
        if (notificationValue != 0) { // notification
            count = 0;
            printf("Reset timer: %02d:%02d:%02d\r\n", count/3600, count%3600/60, count%60);
        }
     }
   }
