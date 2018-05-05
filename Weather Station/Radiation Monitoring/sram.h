
#ifndef _SRAM_H
#define _SRAM_H

#include "includes.h"
#include "sram.h"

typedef enum TYPE_GSM
{
   GSM_ERROR_TYPE = 0,
   CINTERION = 1,
   SIMCOM = 2,
} TYPE_GSM;

enum
{
   SIZE_MOBILE_COUNTRY_CODE = 6,
   SIZE_PIN_CODE = 6,
   SIZE_PASW = 12,
   SIZE_FTP_USER = 16,
   SIZE_MODEM_IDENT = 18,
   SIZE_TEL = 20,
   SIZE_TIME = 20,
   SIZE_IMEI = 20,
   SIZE_NAME_GSM_SOFT = 23,
   SIZE_SCID = 24,
   SIZE_TNUM = 24,
   SIZE_TOKEN = 25,
   LOGIN_PASS_SIZE = 30,
   SIZE_SERV = 32,
   SIZE_SERV_FTP = 40,
   SIZE_USER_REG_INFO = 100,
   SIZE_GPS_POSITION = 100,
};

typedef enum SIM_STATUS
{
   SIM_WAIT = 0,
   HOME_NET = 1,
   ROAMING_NET = 2,
   FIND_NET = 3,
   SIM_ERROR = 10,
   SIM_NO_READY = 11,
   SIM_PIN = 12,
   SIM_PUK = 13,
   SIM_PIN_GUESSING = 14,
   SIM_PIN_NO_SET = 15,
} SIM_STATUS;

//Работа с серверами
typedef enum TYPE_SERVER
{
   FIRST_SERVER = 0,   //первичный
   SECOND_SERVER = 1,   //вторичный
   OTHER_SERVER = 2,   //отличный сервер
   NO_SERVER = 3,
} TYPE_SERVER;

/*РЕЖИМЫ ЭНЕРГОПОТРЕБЛЕНИЯ */
typedef enum
{
   POWER_RUN_MODE = 0,   //обычный режим работы ^_^
   POWER_LOW_PWR1_MODE = 1,   //дремим -_-
   POWER_LOW_PWR2_MODE = 2,   //спим      -_-oO
   POWER_SLEEP_MODE = 3,   //глубокий сон  -_-zZ
} PWR_STATUS;

//Статусы мигания индикационного светодиода.
typedef enum LED_Status
{
   LOAD_DEVICE = 0,   //Загрузка устройства
   FIND_SIM = 1,   //Поиск СИМ карты
   REG_GSM = 2,   //Ожидание регистрации GSM
   SERVER_CONNECT = 3,   //Регистрация в сети GSM пройдена, ожидание подключения к серверу
   NORMAL_RUN = 4,   //Регистрация в сети GSM пройдена, устройство подключено к серверу
   LOW_PWR1 = 5,   //Устройство находится в «спящем режиме» LOW_PWR1
   LOW_PWR2 = 6,   //Устройство находится в «спящем режиме» LOW_PWR2
} LED_Status;

/* СТАТУС ПЕРЕЗАГРУЗКИ И ПРЕДУПРЕЖДЕНИЙ */
typedef enum RESET_STATUS_DEVICE
{
   POWER_ON = 0,
   NO_RESET = 1,   //факта перезагрузки не было.

   /* НАМЕРЕННЫЕ ПЕРЕЗАГРУЗКИ */
   BUTTON_RESET = 2,   //перезагрузка по сигналу/кнопке RESET (не реализовано).
   CMD_RESET = 3,   //перезагрузка по команде прибора.
   WAKE_UP_LOW_PWR1 = 4,   //выход из спящего режима в режиме энергопотребления LOW PWR1.
   WAKE_UP_LOW_PWR2 = 5,   //выход из спящего режима в режиме энергопотребления LOW PWR2.
   WAKE_UP_ALARM = 6,   //выход из спящего режима по напряжению питания SLEEP.
   WAKE_UP_ACCEL = 7,
   UPDATE_FIRM_DEVICE = 8,   //девайс обновил прошивку.
   UPDATE_BASE_DEVICE = 9,   //девайс откатился на базовую прошивку.
   SLEEP_DEVICE = 10,

   /* ПРЕДУПРЕЖДЕНИЯ КОТОРЫЕ ДАЛЕЕ МОГУТ ПОВЛЕЧЬ ПЕРЕЗАГРУЗКИ */
   WARNING_RTOS_HEAP_SIZE_FAIL = 11,   //предупреждение о малом размере кучи FreeRTOS.
   WARNING_RTOS_TASK_NUM_FAIL = 12,   //нарушение количества общих процессов FreeRTOS.
   WARNING_PWR_RESET = 13,   //обнаружен низкий уровень напряжения.
   WARNING_PWR_ONLY_USB = 14,   //идет питание только от USB.
   WARNING_USB_CONNECT = 15,   //подключение девайса к USB.
   WARNING_GPS_DMA_FAIL = 16,   //
   WARNING_HSE_FAIL = 17,   //потеря частоты генерации внешнего кварцевого генератора.
   WARNING_ACCEL_FAIL = 18,   //отказ акселерометра.
   WARNING_EEPROM_FAIL = 19,   //сбой EEPROM

   /* ФАТАЛЬНЫЕ ОШИБКИ */
   ERR_HARD_FAULT = 20,   //выпали в Hard Fault.
   ERR_GSM_FAIL = 21,   //зависание GSM модема.
   ERR_WATCHDOG_RESET = 22,   //сработал Watchdog по зависанию.
   ERR_FLASH_TASK_FAIL = 23,   //зависание процессов FreeRTOS.
   ERR_GSM_TASK_FAIL = 24,   //зависание процессов FreeRTOS.
   ERR_GPS_TASK_FAIL = 25,   //зависание процессов FreeRTOS.
   ERR_PERIPH_TASK_FAIL = 26,   //зависание процессов FreeRTOS.
   ERR_SENSOR_TASK_FAIL = 27,   //зависание процессов FreeRTOS.
   ERR_USB_TASK_FAIL = 28,   //зависание процессов FreeRTOS.

} RESET_STATUS_DEVICE;

//Структура пакета.

typedef __packed struct
{
   uint64_t ucIMEI;   // IMEI GSM модуля
   char strStrIMEI[SIZE_IMEI];   // IMEI GSM модуля в строке
   uint8_t aucGsmCsq[2];   // Уровень CSQ
   char strGsmModemIdentification[SIZE_MODEM_IDENT];   // Идентификатор gsm модема (GSM900 / GSM1800 / UMTS2100 /LTE1800
                                                       // / LTE2600).
   char strGsmModemSoftware[SIZE_NAME_GSM_SOFT];   // Версия прошивки gsm модема.
   TYPE_GSM eGsmType;   // Тип GSM модема.
   int8_t cTemperatur;
   uint32_t usTimePwrGsmNotSim;
   uint32_t usTimePwrGsmSim;
} TGsm;

typedef __packed struct
{
   char strUSER_SCID[SIZE_SCID];   // SCID СИМ карты
   char strOTHER_SCID[SIZE_SCID];   // SCID СИМ карты
   SIM_STATUS eRegStatus;   // роуминг/не роуминг
   char acMobCountCode[SIZE_MOBILE_COUNTRY_CODE];   // Код оператора мобильной связи.
   _Bool bGprsTrue;
} TSim;

typedef __packed struct
{
   TYPE_SERVER
   eConnectNumSer;   //Номер сервера к которому он подключен(0 - FIRST, 1 - SECOND, 3 - AUTOFON, 4 - NO SER).
   _Bool bGprsProfActivate;
   _Bool bWaitGpsModeFm;
} TConnect;

typedef __packed struct
{
   TGsm stGsm;
   TSim stSim;
   TConnect stConnect;
} TRam;
GLOBAL TRam g_stRam;

GLOBAL int g_iFramePaketNumber _EQU(0);

#endif