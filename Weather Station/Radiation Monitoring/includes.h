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
#define SLEEP_MS_300 300
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

#include "sram.h"

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
#include "radmon_general.h"
//**************//

/* RTC */
#include "rtc.h"
#include "calendar.h"
/**************/

/* Radio */
#include "drf.h"
/***********/

/* ONEWIRE */
#include "DS18B20.h"
/***********/

#define RX_BUFFER_SIZE 600   // 300
#define SIZE_IN_DATA_BUF 500   // 200
#define SIZE_OUT_DATA_BUF 600   // 300

GLOBAL char g_asRxBuf[RX_BUFFER_SIZE];   //Буфер для работы парсера GSM.
GLOBAL char g_asCmdBuf[SIZE_IN_DATA_BUF];   //Буфер для работы парсера GSM.

#define MAX_GSM_CONNECT_FAIL 3

/* Настройки GSM & GPRS соединения */
#define DEF_GPRS_RECEIVE_COUNT 5   // ED
#define DEF_GSM_REG_COUNT 30   // ED
#define DEF_GSM_TIMEOUT 30   // SEC
#define DEF_WAIT_SEND_OK 60   // ED                    //Ожидание отправки данных
#define DEF_GPRS_TIMEOUT 30   // SEC
#define DEF_MAX_TIME_FIND_STA 5   // SEC
#define DEF_MIN_COUNT_STA 3   // ED
#define TIMEOUT_FTP_CONNECT 30   //Ждем подключения к FTP

/* GSM */
/*
#include "gsm_hw.h"
#include "gsm_parser.h"
#include "gsm_mc52iT.h"
#include "gsm_check.h"
#include "gsm_info.h"
#include "gsm_gprs.h"
//#include "sms_cmd.h"
//#include "gsm_sms.h"
#include "gsm_code.h"
*/
/**************/

/* ESP8266 */
#include "esp8266.h"
#include "http_general.h"

/* COMMANDS */
#include "cmd_parser.h"
#include "cmd_func.h"

#define DBG_RX_BUFFER_SIZE 64
#define DBG_TX_BUFFER_SIZE 1280
//#define GSM_RX_BUFFER_SIZE 1800  //675
//#define GSM_TX_BUFFER_SIZE 350
#define ESP_RX_BUFFER_SIZE 1280
#define ESP_TX_BUFFER_SIZE 1280

#define GPS_RX_BUFFER_SIZE 1   // 150
#define GPS_TX_BUFFER_SIZE 1   // 64

// buffer
GLOBAL uint8_t g_aucRxBufferUSART1[RX_BUFFER_SIZE1];
GLOBAL uint8_t g_aucRxBufferUSART2[RX_BUFFER_SIZE2];
GLOBAL uint8_t g_aucRxBufferUSART3[RX_BUFFER_SIZE3];
GLOBAL uint8_t g_aucTxBufferUSART1[TX_BUFFER_SIZE1];
GLOBAL uint8_t g_aucTxBufferUSART2[TX_BUFFER_SIZE2];
GLOBAL uint8_t g_aucTxBufferUSART3[TX_BUFFER_SIZE3];

typedef struct
{
   RTC_t stTimeRecords;
   float fDose;
   float fIntTemperatur;
   int iCPM;
} TServer_Data;

typedef struct
{
   int32_t iRealTemperatur;
   _Bool bDataValid;
} TOnewire_Data;

GLOBAL xQueueHandle xQueueServerData _EQU(NULL);   // Очередь данных
GLOBAL xQueueHandle xQueueServDataOW _EQU(NULL);
//**********************//

/* Mutex Initialization */
GLOBAL xSemaphoreHandle mGPS_DATA_ARRIVAL _EQU(NULL);

GLOBAL xTaskHandle CurrentTaskHandle;   // ID текущего процесса(Debug)
GLOBAL char* pNameCurrentTask;   //Имя текущего процесса(Debug)

#endif