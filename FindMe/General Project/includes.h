#ifndef _INCLUDES_H_
#define _INCLUDES_H_

// �������� ������� ��� ��������� ��������� � ����
#define PUBLIC __IO
#define PRIVATE static
#define FORWARD static
#define _EXTERN extern

// ������� ������ �����������
#ifdef __ALLOCATE_NOW__
#define GLOBAL PUBLIC
#define _EQU(x) = (x)
#else
#define GLOBAL _EXTERN
#define _EQU(x)
#endif

#ifdef __ALLOCATE_NOW__
#define V_GLOBAL PUBLIC
#define _EQU(x) = (x)
#else
#define V_GLOBAL _EXTERN __IO
#define _EQU(x)
#endif

// � �������� ������� ����� ���� ���������� �������������� ���� ������ (#define)
#define OK 0
#define FAIL -1

// ����������� �������:
#define BIT(bit) (1UL << (bit))

#define SETBIT(Val, bit) ((Val) |= BIT(bit))
#define CLRBIT(Val, bit) ((Val) &= ~BIT(bit))
#define XORBIT(Val, bit) ((Val) ^= BIT(bit))
#define TSTBIT(Val, bit) ((Val)&BIT(bit))

#define BIT_0 (1 << 0)
#define BIT_1 (1 << 1)
#define BIT_2 (1 << 2)
#define BIT_3 (1 << 3)
#define BIT_4 (1 << 4)
#define BIT_5 (1 << 5)
#define BIT_6 (1 << 6)
#define BIT_7 (1 << 7)
#define BIT_8 (1 << 8)
#define BIT_9 (1 << 9)
#define BIT_10 (1 << 10)
#define BIT_11 (1 << 11)
#define BIT_12 (1 << 12)
#define BIT_13 (1 << 13)
#define BIT_14 (1 << 14)
#define BIT_15 (1 << 15)

#define loop(__a) for(size_t i = 0; i < __a; i++)
#define swap(__a, __b) \
    __a ^= __b; \
    __b ^= __a; \
    __a ^= __b

/* Private typedef BitBand ---------------------------------------------------*/
/* Private define BitBand ----------------------------------------------------*/
#define RAM_BASE 0x20000000
#define RAM_BB_BASE 0x22000000

/* Private macro BitBand -----------------------------------------------------*/
#define Var_ResetBit_BB(VarAddr, BitNumber) \
    (*(__IO uint32_t*)(RAM_BB_BASE | ((VarAddr - RAM_BASE) << 5) | ((BitNumber) << 2)) = 0)

#define Var_SetBit_BB(VarAddr, BitNumber) \
    (*(__IO uint32_t*)(RAM_BB_BASE | ((VarAddr - RAM_BASE) << 5) | ((BitNumber) << 2)) = 1)

#define Var_GetBit_BB(VarAddr, BitNumber) \
    (*(__IO uint32_t*)(RAM_BB_BASE | ((VarAddr - RAM_BASE) << 5) | ((BitNumber) << 2)))
/* ---------------------------------------------------------------------------*/

typedef enum {
    /* ����� ����������� ��� ������� ������ */
    NO_MESS = (0 << 0),
    fBUTTON_MESS_TEL = (1 << 0),    // SMS
    fMOVE_MESS_TEL = (1 << 1),
    fSTOP_MESS_TEL = (1 << 2),
    fFIND_MESS_TEL = (1 << 3),
    fPOWER_MESS_TEL = (1 << 4),
    fWAKEUP_MESS_TEL = (1 << 5),

    fBUTTON_CALL_TEL = (1 << 7)    // CALL
} TYPE_MESSAGE;

/* ������ ������������ � �������������� */
typedef enum RESET_STATUS_DEVICE {
    NO_RESET = 0,    //����� ������������ �� ����.
    POWER_ON = 1,

    /* ���������� ������������ */
    BUTTON_RESET = 2,          //������������ �� �������/������ RESET .
    CMD_RESET = 3,             //������������ �� ������� �������.
    WAKE_UP_LOW_PWR1 = 4,      //����� �� ������� ������ � ������ ����������������� LOW PWR1.
    WAKE_UP_LOW_PWR2 = 5,      //����� �� ������� ������ � ������ ����������������� LOW PWR2.
    WAKE_UP_ALARM = 6,         //����� �� ������� ������ �� �������.
    WAKE_UP_ACCEL = 7,         //����� �� ������� ������ �� �������������.
    WAKE_UP_STOP = 8,          //������ �������� ���������.
    UPDATE_FIRM_DEVICE = 9,    //������ ������� ��������.
    LOW_POWER = 10,            //����� �� ������� ������ �� ������� ���������� �������.

    /* �������������� ������� ����� ����� ������� ������������ */
    WARNING_RTOS_HEAP_SIZE_FAIL = 11,    //�������������� � ����� ������� ���� FreeRTOS.
    WARNING_RTOS_TASK_NUM_FAIL = 12,     //��������� ���������� ����� ��������� FreeRTOS.
    WARNING_PWR_RESET = 13,              //��������� ������ ������� ����������.
    WARNING_PWR_ONLY_USB = 14,           //���� ������� ������ �� USB.
    WARNING_USB_CONNECT = 15,            //����������� ������� � USB.
    WARNING_GPS_DMA_FAIL = 16,           //
    WARNING_HSE_FAIL = 17,               //������ ������� ��������� �������� ���������� ����������.
    WARNING_ACCEL_FAIL = 18,             //����� �������������.
    WARNING_EEPROM_FAIL = 19,            //���� EEPROM

    /* ��������� ������ */
    ERR_HARD_FAULT = 20,        //������ � Hard Fault.
    ERR_GSM_FAIL = 21,          //������ GSM ������.
    ERR_WATCHDOG_RESET = 22,    //�������� Watchdog �� ���������.
} RESET_STATUS_DEVICE;

typedef enum {
    INIT_ALL = 0,
    INIT_MAIN,
    INIT_ADD,
} INIT_CONFIG;

typedef enum VALUE {
    OFF = 0,
    ON,
    HARD_RESET,
    SLEEP,
    WAKE_UP
} VALUE;

#define RUN_RESET 0
#define ONE_RESET 1
#define TWO_RESET 2

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <intrinsics.h>
#include <assert.h>
#include "system_stm32l1xx.h"
#include "stm32l1xx.h"
#include "arm_comm.h"
#include "stdtypes.h"

/* HW includes */
#ifdef FM3
#include "platform_config_fm3.h"
#endif
#ifdef FM4
#include "platform_config_fm4.h"
#endif
#include "power.h"
#include "test_device.h"

/* DRIVER */
#include "rtc.h"
#include "calendar.h"
#include "SPI.h"
#ifdef FM3
#include "EXT_FLASH_FM3.h"
#endif
#ifdef FM4
#include "EXT_FLASH_FM4.h"
#endif
#include "INT_FLASH.h"
#include "UART.h"
#include "ACCEL2.h"
#include "TIMER.h"
#include "ADC_1.h"
/**************/

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"
/**************/

/* GSM */
#include "gsm_general.h"
#include "gsm_parser.h"
#include "gsm_mc52iT.h"
#include "gsm_info.h"
#include "gsm_gprs.h"
#include "fm911.h"
#include "ion.h"

/* SMS */
#include "sms_cmd.h"
#include "gsm_sms.h"
/*************/

/* PROTOCOL */
#include "protocol_general.h"
#include "protocol_realize.h"
#include "findme_911.h"
/**************/

/* GPS */
//#include "gps_parser_ver1.h"
#include "gps_parser_ver2.h"
#include "gps_tools.h"
#include "gps_general.h"
#include "agps_protocol.h"
/*************/

/* DEBUG */
#include "debug.h"
#include "periph_general.h"

/* FLASH & ARCHIVE */
#include "flash_general.h"
#include "flash_archive.h"
#include "INT_FLASH.h"

/* EEPROM */
#include "int_eeprom.h"
#include "eeprom.h"
#include "ram.h"
/**************/

/* TOOLS */
#include "crc16.h"
#include "tools.h"
//#include "printf-stdarg.h"
/**************/

/* CONFIG COMMAND */
#include "cmd_parser.h"
#include "cmd_func.h"
/**************/

/* GLOBAL CONFIG */
#include "global_ram.h"
#include "global_eep.h"

#include "gsm_http.h"
#include "drive_simple.h"

#define RX_BUFFER_SIZE 512       // 300
#define SIZE_IN_DATA_BUF 400     // 200
#define SIZE_OUT_DATA_BUF 520    // 300

#define SIZE_FLASH_BUF 512

GLOBAL char g_asInpDataFrameBuffer[SIZE_IN_DATA_BUF];
GLOBAL char g_aucOutDataFrameBuffer[SIZE_OUT_DATA_BUF];
GLOBAL char g_asRxBuf[RX_BUFFER_SIZE];       //����� ��� ������ ������� GSM.
GLOBAL char g_asCmdBuf[SIZE_IN_DATA_BUF];    //����� ��� ������ ������� GSM

#define MAX_PARAM_CONFIG_VALUE 256

#define DBG_RX_BUFFER_SIZE 1
#define DBG_TX_BUFFER_SIZE 300    // 350

#define GSM_RX_BUFFER_SIZE 1150    // 1200

#define GSM_TX_BUFFER_SIZE 400    // 350

#define GPS_RX_BUFFER_SIZE 1     // 150
#define GPS_TX_BUFFER_SIZE 64    // 64
//**********************//

//// ----- USART ---- ////
#define RX_BUFFER_SIZE1 DBG_RX_BUFFER_SIZE
#define TX_BUFFER_SIZE1 DBG_TX_BUFFER_SIZE

#define RX_BUFFER_SIZE2 GSM_RX_BUFFER_SIZE
#define TX_BUFFER_SIZE2 GSM_TX_BUFFER_SIZE

#define RX_BUFFER_SIZE3 GPS_RX_BUFFER_SIZE
#define TX_BUFFER_SIZE3 GPS_TX_BUFFER_SIZE
//**********************//

#define MAX_COUNT_SIM_FIRST_ERROR 2     // ������������ ���������� ������ � ������ �� �������� SIM
#define MAX_COUNT_SIM_SECOND_ERROR 2    // ������������ ���������� ������ � ������ �� ��������� SIM
#define MAX_TIME_CONTACT_SIM_SECOND \
    2629743    // ������������ ���������� ������� ����� ���� � ������ ��������� SIM (1 ����� (30.44 ����))

#define MAX_GSM_CONNECT_FAIL 2    // ED
#define TIMEOUT_OPEN_GPRS 10      // SEC

/* ��������� GSM & GPRS ���������� */

#define DEF_GSM_TIMEOUT 30          // SEC
#define DEF_WAIT_SEND_OK 60         // ED                    //�������� �������� ������
#define DEF_GPRS_TIMEOUT 30         // SEC
#define DEF_MAX_TIME_FIND_LBS 60    // SEC

/* ION FM MODE */
#define DEF_TIMEOUT_LOW_POWER_MODE1 \
    10    // MIN        15���. \
          //����� �������� ��
#define DEF_SLEEP_LOW_POWER_MODE1 \
    20    // MIN        1 ���  // \
          //����� ����������� � ������� �� ������. \
          //��������� � ����� LOW PWR2 �����
#define DEF_TIMEOUT_LOW_POWER_MODE2 \
    30    // MIN        2 ���. \
          //�������� ��
#define DEF_SLEEP_LOW_POWER_MODE2 \
    60    // MIN       12���. \
          //����� ����������� � ������� �� ������.

#define DEF_FLASH_DATA_LEN 1024    // BYTE

/* FINDME MODE */
#define DEF_TIME_SLEEP_STANDART_DEVICE 1    //����� ������ � ����������� ������ � ������
#define MIN_VAL_SLEEP_TIME_STANDART 1       //� ���
#define MAX_VAL_SLEEP_TIME_STANDART 3

#define DEF_TIME_SLEEP_FIND_DEVICE 60    //����� ������ � ������ ������ � �������
#define MIN_VAL_SLEEP_TIME_FIND 5
#define MAX_VAL_SLEEP_TIME_FIND 5760

#define DEF_TIME_GPS_WAIT (10 * 60)    // 10 min
#define MIN_VAL_FIND_GPS_SAT 60
#define MAX_VAL_FIND_GPS_SAT 3600

/* �������� �� ���������������� ����� � ������ ������� */
#define DEF_TIME1_RECONNECT (10 * 60)         // 10 ���
#define DEF_TIME2_RECONNECT (1 * 60 * 60)     // 1 ���
#define DEF_TIME3_RECONNECT (3 * 60 * 60)     // 3 ����
#define DEF_TIME4_RECONNECT (5 * 60 * 60)     // 5 �����
#define DEF_TIME5_RECONNECT (24 * 60 * 60)    // 1 �����

#define DEF_TIME_WAIT_SMS (5 * 60)    // SEC

#define MIN_VAL_LOW_POWER1 5
#define MIN_VAL_LOW_POWER2 10

#define MAX_VAL_LOW_POWER1 2880
#define MAX_VAL_LOW_POWER2 5760

#define MIN_VAL_RT 0
#define MAX_VAL_RT 3600

#define MIN_VAL_FTIME 0
#define MAX_VAL_FTIME 65535

#define MIN_VAL_SIZE_PASW 4
#define MIN_VAL_SIZE_TEL 8

/* ����� GPS ������ � ����� */
#define DEF_GPS_REAL_TIME_MODE 60    // SEC   //����� �������� ����� ��������� � ��������

#define DEF_GPS_RECORD_ACCEL_MODE1 OFF    //������ �� �������� � ������ �����
#define DEF_GPS_RECORD_TIME_MODE1 120     // SEC
#define MIN_SPEED_GPS 20                  // SPEED IN KM
#define DEF_GPS_COURSE 15                 // GRAD
#define DEF_GPS_DISTANCE 100              // METR

/* GPIO */
#define DEF_GPIO_TIME 60    // SEC
#define DELTA_V 100U

#define DEF_HDOP_FIX_COORD 6    // HDOP

/* ACCEL */
#define DEF_ACCEL_SENSITIVITY 3               // ED
#define DEF_ACCEL_TIME_CURR_STATE (3 * 60)    // SEC

/* USER */
#define DEF_PASSWORD "123456"

#define DEF_SIM_PIN "0000"    // PIN CODE

#define DEF_NUM_SERVER FIRST_SERVER
#ifdef FM3
#define DEF_FIRST_SERVER "srv.irzonline.ru:18000"    // NAME
#endif
#ifdef FM4
#define DEF_FIRST_SERVER "srv.irzonline.ru:18002"    // NAME
#endif

#define DEF_SECOND_SERVER "911.fm:20000"    // NAME
//#define DEF_SECOND_SERVER               "94.19.156.130:8888"

#define DEF_HTTP_SERVER \
    "online.irz.net/f.php?f="    //��������� ����� ������ ����� ������� �������� &i=353437069574298&b=0//1458917674

#define DEF_TIME_JAMMING_DETECTED 60    // SEC

#define STR_NULL ""

#define DELTA_TIME_OFFSET 30

// buffer
GLOBAL u8 g_aucRxBufferUSART1[RX_BUFFER_SIZE1];
GLOBAL u8 g_aucTxBufferUSART1[TX_BUFFER_SIZE1];

GLOBAL u8 g_aucRxBufferUSART2[RX_BUFFER_SIZE2];
GLOBAL u8 g_aucTxBufferUSART2[TX_BUFFER_SIZE2];

GLOBAL u8 g_aucRxBufferUSART3[RX_BUFFER_SIZE3];
GLOBAL u8 g_aucTxBufferUSART3[TX_BUFFER_SIZE3];

// semaphore and mutex
GLOBAL xSemaphoreHandle mGPS_DATA_ARRIVAL _EQU(NULL);

GLOBAL xSemaphoreHandle sBinSemFLASH_BUFF _EQU(NULL);    //������ � ������
GLOBAL xSemaphoreHandle sBinSemFLASH _EQU(NULL);         //������ � ������
GLOBAL xSemaphoreHandle sBinSemDATA_WRITE _EQU(NULL);    //������ � ������
GLOBAL xSemaphoreHandle sBinSemUSART _EQU(NULL);         //������ � USART

GLOBAL xSemaphoreHandle mINIT_GPS_MODULE _EQU(NULL);
GLOBAL xSemaphoreHandle mDEINIT_GPS_MODULE _EQU(NULL);

GLOBAL xSemaphoreHandle sBinSPI _EQU(NULL);

GLOBAL xSemaphoreHandle mFLASH_SAVE_DATA_FM _EQU(NULL);
GLOBAL xSemaphoreHandle mFLASH_COMPLETE _EQU(NULL);

GLOBAL QueueHandle_t xAccelQueue _EQU(NULL);
GLOBAL QueueHandle_t xFlashQueue _EQU(NULL);
GLOBAL QueueHandle_t xEepromQueue _EQU(NULL);

GLOBAL char InpDataBuffer[SIZE_IN_DATA_BUF];

GLOBAL uint32_t g_uiPacStatDevTime _EQU(NULL);

/* BitBand -------------------------------------------------------------------*/
GLOBAL uint32_t VarAddr _EQU(NULL);
GLOBAL uint32_t VarBitValue _EQU(NULL);
GLOBAL uint32_t Var _EQU(NULL);
#endif