#ifndef _GSM_PARSER_H_
#define _GSM_PARSER_H_

#include "includes.h"
#include "sram.h"

typedef enum M_INFO
{
   M_OK = 0,
   M_ERROR = 1,
   M_SIS = 2,
   M_SISR = 3,
   M_SISW = 4,
   M_SISI = 5,
   M_SICI = 6,
   M_SYSSTART = 7,
   M_SMSO = 8,
   M_SCKS = 9,
   M_SCTM = 10,
   M_SCID = 11,
   M_SHUTDOWN = 12,
   M_CSQ = 13,
   M_SIM_PIN = 14,
   M_SIM_PUK = 15,
   M_SIM_READY = 16,
   M_CREG = 17,
   M_COPS = 18,
   M_CMTI_MT = 19,
   M_CMTI_ME = 20,
   M_CMTI_SM = 21,
   M_CME_ERROR = 22,
   M_SMGL = 23,
   M_SMGR = 24,
   M_RING = 25,
   M_PROMPT = 26,
   M_UVOLTAGE = 27,
   M_OVOLTAGE = 28,
   M_SMOND = 29,
   M_SIND_NITZ = 30,
   M_ABORTED = 31,
   M_CUSD = 32,
   M_NO_CARRIER = 33,
   M_SPIC = 34,
   M_P_CMGL = 35,
   M_CGATT = 36,
   M_0_CONNECT_OK = 37,
   M_1_CONNECT_OK = 38,
   M_0_SEND_OK = 39,
   M_1_SEND_OK = 40,
   M_RECEIVE = 41,
   M_0_CLOSE_OK = 42,
   M_1_CLOSE_OK = 43,
   M_0_CLOSED = 44,
   M_1_CLOSED = 45,
   M_FTPGET = 46,
   M_JAMMING = 47,
   M_HTTPACTION = 48,
   M_HTTPREAD = 49,
   M_BTPAIRING = 50,
   M_BTCONNECTING = 51,
   M_BTCONNECT = 52,
   M_BTSPPDATA = 53,
   M_RDY = 54,
   M_BTDISCONN = 55,
   M_CALL_READY = 56,
   M_TOTAL = 57,

   M_STRING = 0xF0,
   M_SYSTEM = 0xF1,
   M_OTHER_PIN = 0xF2,
   M_ECHO = 0xF3,
   M_DMA_ERR = 0xF4,
   M_OTHER = 0xFF
} M_INFO;

typedef enum SRV_STATE
{
   ALLOCATED = 2,   //Сервис не открыт, но готов к конфигурации.
   CONNECTING = 3,   //Подключен.
   UP = 4,   //Сервис выполняет свое предназначение, процесс передачи данных и основные функции.
   CLOSING = 5,   //Интернет-сервис закрывает сетевое соединение.
   DOWN = 6   // IP-подключение было закрыто.
} SRV_STATE;

// информация по одной GSM станции
typedef struct
{
   u16 mcc;
   u16 mnc;
   u32 lac;
   u32 cell;
   u16 rxlev;
} TS_STATION;

// информация по всем видимым станциям GSM (до 7 штук)
typedef struct
{
   TS_STATION station[20];
   u8 ta;   //Растояние от одной GSM вышки(только для BGS2).
} TS_MOND;

// информация об одной переменной в сообщении от GSM модуля (число или указатель на строку)
typedef union
{
   int32_t var;
   char* str;
} TS_VAR;

// информация до 8 переменных в сообщении от GSM модуля
typedef struct
{
   union
   {
      int32_t var;
      char* str;
   };
   u8 size;
} TS_DATA;

// информация о GSM операторах связи
typedef struct
{
   u32 opName;
   u8 status;
} TS_COPS;

typedef struct
{
   u8 m_type;
   u8 m_echo;
   u32 count;
   union
   {
      TS_DATA msg[8];
      TS_COPS ops[8];
   };
   TS_MOND* inf;
   int8_t s8Temperatur;
} GSM_INFO;

GLOBAL TS_MOND g_base_station;
#define INIT_GSM_INFO(x) x.inf = &g_base_station

//структура описывающее состояние уровень сигнала gsm, наличие gprs, соединения с сервером и т.п.
typedef struct
{
   u8 ucCSQ;   //сила сигнала
   u8 ucCREG[2];   //статус gprs
   u8 srv_connection;   //статус подключения к серверу

} GSM_STAT;

typedef struct
{
   u8 m_size;
   u8 v_total;
   u8 v_type;
   u8 msg[21];
} MSG_TABLE;

GLOBAL GSM_INFO g_stDataGsmInfo;

int gsm_parser(const char* cmd, GSM_INFO* a, char* buf, int b_size, uint32_t wait);
u32 calculate(u8 sign, u8 base, u8 parser_byte, u32 res);

#endif
