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

/* USB */
#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "hw_config.h"
#include "usb_pwr.h"
//**************//

/* DHT 11 */
#include "dht11.h"
#include "dht22.h"
#include "humidity_general.h"
//**********************//

/* RTC */
#include "rtc.h"
#include "calendar.h"
/**************/

/* LCD */
#include "LCD.h"
#include "lcd_general.h"
/***********/

/* BMP 180 */
#include "bmp180.h"
#include "pressure_general.h"

/* GPS Code */
#include "gps_general.h"
#include "gps_parser_ver2.h"
#include "gps_tools.h"

/* BUTTON Code */
#include "button_general.h"

typedef struct
{
   uint16_t usRealHumidity;   //
   int16_t sRealTemperatur;   //
   _Bool bDataValid;
} THumidity_Data;

typedef struct
{
   int32_t iRealTemperatur;
   int32_t iRealPressurePa;
   int32_t iRealPressureHg;
   _Bool bDataValid;
} TPressure_Data;

typedef struct
{
   float fLatitude;
   float fLongitude;
   _Bool bGpsDataValid;
   RTC_t stTimeRecords;   // 1
   int32_t iPressurePa;   // 2
   int32_t iPressureHg;   // 3
   float fIntTemperatur;   // 4
   float fExtTemperatur;
   uint16_t usExtHumidity;   // 6
} TServer_Data;

/* GPS */
#define GPS_RX_BUFFER_SIZE 1   // 150
#define GPS_TX_BUFFER_SIZE 64   // 64

// buffer
GLOBAL uint8_t g_aucRxBufferUSART1[RX_BUFFER_SIZE1];
GLOBAL uint8_t g_aucRxBufferUSART2[RX_BUFFER_SIZE2];
GLOBAL uint8_t g_aucRxBufferUSART3[RX_BUFFER_SIZE3];
GLOBAL uint8_t g_aucTxBufferUSART1[TX_BUFFER_SIZE1];
GLOBAL uint8_t g_aucTxBufferUSART2[TX_BUFFER_SIZE2];
GLOBAL uint8_t g_aucTxBufferUSART3[TX_BUFFER_SIZE3];

GLOBAL GPS_INFO stGpsData;   //Структура GPS.

GLOBAL xQueueHandle xQueueServerData _EQU(NULL);   // Очередь данных внешнему серверу(например thingspeak.com)
GLOBAL xQueueHandle xQueueExtHumidityForLcd _EQU(NULL);   // Очередь данных о влажности.
GLOBAL xQueueHandle xQueueIntHumidityForLcd _EQU(NULL);   // Очередь данных о влажности.
GLOBAL xQueueHandle xQueuePressureForLcd _EQU(NULL);   // Очередь данных о давлении.

//**********************//

/* Mutex Initialization */
GLOBAL xSemaphoreHandle mINIT_WIFI_ESP _EQU(NULL);
GLOBAL xSemaphoreHandle mSEND_DATA_SERVER _EQU(NULL);

GLOBAL xSemaphoreHandle mGPS_DATA_ARRIVAL _EQU(NULL);
GLOBAL xSemaphoreHandle mLCD_LED_INDICATING _EQU(NULL);
GLOBAL xSemaphoreHandle mLCD_GPS_INDICATING _EQU(NULL);

GLOBAL xTaskHandle CurrentTaskHandle;   // ID текущего процесса(Debug)
GLOBAL char* pNameCurrentTask;   //Имя текущего процесса(Debug)

#endif