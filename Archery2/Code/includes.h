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

#define loop(__a) for(int i = 0; i < __a; i++)
#define swap(__a, __b) \
    __a ^= __b; \
    __b ^= __a; \
    __a ^= __b

/* Standart functions */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <intrinsics.h>
#include <assert.h>
/*----------------------------------------------------------------------------*/

/* Device functions */
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "arm_comm.h"
#include "stm32f10x_it.h"
/*----------------------------------------------------------------------------*/

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"
/*----------------------------------------------------------------------------*/

/* USB -----------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "hw_config.h"
#include "usb_pwr.h"
/*----------------------------------------------------------------------------*/

#include "platform_config.h"

#include "uart.h"

/* Driver */
#include "adc.h"
#include "adxl345.h"
#include "ssd1306.h"

#include "display.h"

/**/
#include "accel.h"

/* Display */
#include "fonts.h"
#include "img.h"

#include "debug.h"

GLOBAL QueueHandle_t xBuzQueue;

GLOBAL xSemaphoreHandle sBinSemDbg _EQU(NULL);    // Мьютекс распределяющий вывод сообщений в отладочный интерфейс
GLOBAL xSemaphoreHandle sBinSemReservDbgBuf
    _EQU(NULL);    // Мьютекс обозначающий, что есть не выгруженные данные в отладочный интерфейс

GLOBAL xSemaphoreHandle sBinSemAccelInterrupt _EQU(NULL);

#endif
