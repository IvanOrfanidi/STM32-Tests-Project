#ifndef _INCLUDES_H_
#define _INCLUDES_H_

// Полезные макросы для уточнения намерений в коде
#define PUBLIC __IO
#define PRIVATE static
#define FORWARD static
#define _EXTERN extern

// Правило одного определения
#ifdef __ALLOCATE_NOW__
#define GLOBAL PUBLIC
#define _EQU(x) = (x)
#else
#define GLOBAL _EXTERN
#define _EQU(x)
#endif

// В описании функции могут быть определены дополнительные коды ошибок (#define)
#define OK 0
#define FAIL -1

// Стандартные макросы:
#define BIT(bit) (1UL << (bit))

#define SETBIT(Val, bit) ((Val) |= BIT(bit))
#define CLRBIT(Val, bit) ((Val) &= ~BIT(bit))
#define XORBIT(Val, bit) ((Val) ^= BIT(bit))
#define TSTBIT(Val, bit) ((Val)&BIT(bit))

#define _delay_ms(delay) osDelay(delay)

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <intrinsics.h>
#include <assert.h>

// Device functions //
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "arm_comm.h"
#include "stm32f10x_it.h"
//**************//

#include "platform_config.h"

// FreeRTOS includes //
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"
//**************//

// Drivers //
#include "USART.h"
//**************//

/* DEBUG */
#include "debug.h"
//**************//

/* USB */
#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "hw_config.h"
#include "usb_pwr.h"
//**************//

/* BMP 180 */
#include "bmp180.h"
#include "pressure_general.h"

typedef __packed struct
{
    int32_t iRealTemperatur;
    int32_t iRealPressurePa;
    int32_t iRealPressureHg;
    _Bool bDataValid;
} TPressure_Data;

// buffer
GLOBAL uint8_t g_aucRxBufferUSART1[RX_BUFFER_SIZE1];
GLOBAL uint8_t g_aucRxBufferUSART2[RX_BUFFER_SIZE2];
GLOBAL uint8_t g_aucRxBufferUSART3[RX_BUFFER_SIZE3];
GLOBAL uint8_t g_aucTxBufferUSART1[TX_BUFFER_SIZE1];
GLOBAL uint8_t g_aucTxBufferUSART2[TX_BUFFER_SIZE2];
GLOBAL uint8_t g_aucTxBufferUSART3[TX_BUFFER_SIZE3];

GLOBAL xQueueHandle xQueuePressureDataToUsart;    // Очередь данных о давлении.
/**********************/

GLOBAL xTaskHandle CurrentTaskHandle;    // ID текущего процесса(Debug)
GLOBAL char* pNameCurrentTask;           //Имя текущего процесса(Debug)

#endif