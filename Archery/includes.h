#ifndef _INCLUDES_H_
#define _INCLUDES_H_

// Полезные макросы для уточнения намерений в коде
#define PUBLIC __IO
#define PRIVATE static
#define FORWARD static
#define _EXTERN extern

// Правило одного определения
#ifdef __ALLOCATE_NOW__
#   define GLOBAL PUBLIC
#   define _EQU(x) = (x)
#else
#   define GLOBAL _EXTERN
#   define _EQU(x)
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

#define SLEEP_MS_60000 60000
#define SLEEP_MS_10000 10000
#define SLEEP_MS_5000 5000
#define SLEEP_MS_4000 4000
#define SLEEP_MS_3000 3000
#define SLEEP_MS_2500 2500
#define SLEEP_MS_2000 2000
#define SLEEP_MS_1000 1000
#define SLEEP_MS_750 750
#define SLEEP_MS_500 500
#define SLEEP_MS_250 250
#define SLEEP_MS_200 200
#define SLEEP_MS_150 150
#define SLEEP_MS_100 100
#define SLEEP_MS_50 50
#define SLEEP_MS_10 10
#define SLEEP_MS_5 5
#define SLEEP_MS_1 1

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

/* RTC */
#include "rtc.h"
#include "calendar.h"
/**************/

/* TM1637 */
#include "tm1637.h"
/**************/

/* */
#include "adxl345.h"
#include "accel.h"

// buffer
GLOBAL uint8_t g_aucRxBufferUSART1[RX_BUFFER_SIZE1];
GLOBAL uint8_t g_aucRxBufferUSART2[RX_BUFFER_SIZE2];
GLOBAL uint8_t g_aucRxBufferUSART3[RX_BUFFER_SIZE3];
GLOBAL uint8_t g_aucTxBufferUSART1[TX_BUFFER_SIZE1];
GLOBAL uint8_t g_aucTxBufferUSART2[TX_BUFFER_SIZE2];
GLOBAL uint8_t g_aucTxBufferUSART3[TX_BUFFER_SIZE3];

//**********************//

typedef enum
{
   CHARGING_OFF = 0,
   CHARGING_ON,
   CHARGING_COMPLET
} TYPE_CHARGING;
GLOBAL TYPE_CHARGING g_eStatusCharging _EQU(CHARGING_OFF);

typedef __packed struct
{
   int16_t sValueAxisX;
   int16_t sValueAxisY;
   int16_t sValueAxisZ;
   u8 ucInterrupt;
   _Bool bDataValid;
} TAccel_Data;
GLOBAL TAccel_Data g_stAccelData;

GLOBAL xTaskHandle CurrentTaskHandle;   // ID текущего процесса(Debug)
GLOBAL char* pNameCurrentTask;   //Имя текущего процесса(Debug)

void IncreaseCountShot(void);

#endif