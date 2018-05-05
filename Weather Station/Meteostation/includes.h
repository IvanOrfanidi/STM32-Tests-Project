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
//**************//

// FreeRTOS includes //
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"
//**************//

#include "platform_config.h"
#include "freertos_general.h"

// Drivers //
#include "rtc.h"
#include "calendar.h"
#include "USART.h"
#include "LCD.h"
#include "timer.h"

/* USB */
#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "hw_config.h"
#include "usb_pwr.h"
//**************//

// Ethernet
#include "enc28j60.h"
#include "hello-world.h"
#include "uip.h"
#include "uip_arp.h"
#include "ethernet_general.h"

// DHT 11 //
#include "dht11.h"
#include "humidity_general.h"

// BMP 180 //
#include "bmp180.h"
#include "pressure_general.h"

#include "adc_general.h"

// SD Card

#include "integer.h"
#include "ff.h"
#include "diskio.h"
#include "sdcard.h"
#include "sdcard_general.h"

// Onewire
#include "DS18B20.h"
#include "onewire_general.h"
//**************//

// LCD //
#include "lcd_general.h"

/* DEBUG */
#include "debug.h"
//**************//

typedef __packed struct
{
   int32_t iRealTemperatur;
   _Bool bDataValid;
} TOnewire_Data;

typedef __packed struct
{
   int32_t iRealTemperatur;
   int32_t iRealPressurePa;
   int32_t iRealPressureHg;
   _Bool bDataValid;
} TPressure_Data;

typedef __packed struct
{
   uint16_t usRealTemperatur;   //
   uint16_t usRealHumidity;   //
   _Bool bDataValid;
} THumidity_Data;

typedef struct
{
   uint16_t usValue1;
   uint16_t usValue2;
   uint16_t usValue3;
} ADCmeasCFG;

// ID Task
GLOBAL xTaskHandle xHandleAdcTask;
GLOBAL xTaskHandle xHandleSDCard;
GLOBAL xTaskHandle xHandleOnewireTask;
GLOBAL xTaskHandle xHandlePressureTask;
GLOBAL xTaskHandle xHandleHumidityTask;
GLOBAL xTaskHandle xHandleButTask;
GLOBAL xTaskHandle xHandleLcdTask;
GLOBAL xTaskHandle xHandleDebugTask;
//**********************//

GLOBAL xQueueHandle xQueueAdcDataToUsart;   //Очередь данных с АЦП.

GLOBAL xQueueHandle xQueuePressureDataToLcd;   // Очередь данных о давлении.
GLOBAL xQueueHandle xQueuePressureDataToUsart;   // Очередь данных о давлении.

GLOBAL xQueueHandle xQueueHumidityDataToLcd;   // Очередь данных о влажности.
GLOBAL xQueueHandle xQueueHumidityDataToUsart;   // Очередь данных о влажности.

GLOBAL xQueueHandle xQueueOnewireDataToUsart;   // Очередь данных о температуре.
GLOBAL xQueueHandle xQueueOnewireDataToLcd;   // Очередь данных о температуре.
GLOBAL xQueueHandle xQueueOnewireDataToSdcard;

//**********************//

GLOBAL xTaskHandle CurrentTaskHandle;   // ID текущего процесса(Debug)
GLOBAL char* pNameCurrentTask;   //Имя текущего процесса(Debug)

// buffer
GLOBAL uint8_t g_aucRxBufferUSART1[RX_BUFFER_SIZE1];
GLOBAL uint8_t g_aucRxBufferUSART2[RX_BUFFER_SIZE2];
GLOBAL uint8_t g_aucRxBufferUSART3[RX_BUFFER_SIZE3];
GLOBAL uint8_t g_aucTxBufferUSART1[TX_BUFFER_SIZE1];
GLOBAL uint8_t g_aucTxBufferUSART2[TX_BUFFER_SIZE2];
GLOBAL uint8_t g_aucTxBufferUSART3[TX_BUFFER_SIZE3];

#define TIM4_PERIOD 1000
GLOBAL uint16_t usTIMET4_CCR1_Val _EQU(500);

#endif