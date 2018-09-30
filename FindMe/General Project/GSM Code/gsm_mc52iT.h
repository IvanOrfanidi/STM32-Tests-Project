#ifndef _GSM_MC52IT_H_
#define _GSM_MC52IT_H_

#include "includes.h"
#include "gsm_parser.h"
#include "eeprom.h"
#include "ram.h"

/* ВНИМАНИЕ: КОМАНДЫ ПИСАТЬ ТОЛЬКО МАЛЫМИ ПРОПИСНЫМИ БУКВАМИ */
#define ATZ "atz"
#define AT_CONFIG_LED "at+csgs=1"   //Миганее GPIO LED для индикации GSM сети и сервиса
#define AT_SCID "at+ccid"   //Запрос индификационного номера сим карты
#define AT_SMGL "at+cmgl=4"   //Читаем все СМС
#define AT_SPIC "at+spic"   //Проверяем сколько раз пытались вводить пинкод

#define AT_CGATT "at+cgatt?"   //Проверяем сервис GPRS (SIM800)
#define AT_CIPMUX "at+cipmux=1"   //включить мультирежим – чтобы подключать неск серверов
#define AT_SEND "at+cipsend="   //отправить на профиль данные
#define AT_SISR_FTP "at+ftpget="   //Принемаем данные ftp

#define ADD_SEND_PAR ""
#define AT_SMSO "at+cpowd=0"   //выключить питание
#define AT_CIPCLOSE "at+cipclose="   //зкрыть сокет
#define AT_CIPSTATUS "at+cipstatus="
#define AT_CNETSCAN1 "at+cnetscan=1"   //Получаем более полную инфо о gsm станциях
#define AT_SMOND "at+cnetscan"   //Поиск базовых станций

#define AT_CIPHEAD1 "at+ciphead=1"   //включить сообщение о количестве байт принятых по GPRS перед самими данными

#define AT_CMGD "at+cmgda="

/* КОМАНДЫ СПЕЦИФИЧНЫЕ ТОЛЬКО ДЛЯ SIM800 */
#define AT_BT_ON "at+btpower=1"   //Включение Bluetooth
#define AT_BT_OFF "at+btpower=0"   //Отключение Bluetooth

#define AT_JD_ON "at+sjdr=1,1"   //Включение режима детектирования глушения GSM сигнала и слать статус в UART.
#define AT_JD_OFF "at+sjdr=0"   //Отключение режима детектирования глушения GSM сигнала

#define AT_CENG "at+ceng?"

#define AT_CPIN "at+cpin?"

#define SMS_PDU
//#define SMS_TEXT

//#define CLI_ENABLE //АОН ВКЛЮЧЕН
#define MULTI_IP_CONNECTION_ENABLE   //включить мультирежим – чтобы подключать неск серверов

#ifndef ADD_INFO_GSM_MOD
#   define ADD_INFO_GSM_MOD 0
#endif

#define MC_COUNT 3

typedef enum RET_INFO
{
   RET_OK = 0,
   ERR_CMD,
   ERR_POWER,   // 2
   ERR_RESTART,
   ERR_TIMEOUT,
   ERR_SIMCART,
   ERR_SIM_NO_READY,
   ERR_GSMNET,
   ERR_APN,
   ERR_GPRS_ACTIVATE,
   ERR_GPRS_TIMEOUT,   // 10
   ERR_GPRS_SEND,   // 11
   ERR_SERVER_CONNECT,
   ERR_SERVER_NO_ANSWER,
   ERR_EE_DATA,
   ERR_TEMPERATURE,
   ERR_ABORTED,
   IT_IS_OK,
   IT_IS_BREAK,
   HACK_TRANSMIT_DATA,
   ERR_SIM_LITTLE_PIN,
   RET_GPRS_OK,
   RET_GPRS_SEND_OK,
   RET_GPRS_RECEIVE_OK,
   ERR_WRITE_FLASH,   // 25
   ERR_DOWNLOAD_FIRMWARE,
   ERR_DMA,
   RET_FTP_OK,
   RET_JD_OK,

   ERR_BT_SEND,   //Ошибка отправки данных по BT
   RET_BT_WAIT_DATA,   //Данных по BT небыло
   RET_BT_TIMEOUT,   //Истек таймаут соединения по BT
   RET_BT_DISCONN,   //Соединение BT закрылось

   RET_CLOSE0,
   RET_CLOSE1,

   RET_HTTP_OK,
   RET_HOME_NET = 0x31,
   RET_ROAMING_NET = 0x35,
   RET_HTTP_SERV,
} RET_INFO;

typedef enum GSM_COMMAND
{
   GSM_PWRON = 0,
   GSM_PWROFF = 1,
   GSM_SLEEP = 2,
   GSM_WAKEUP = 3,
   GSM_LOCK = 4
} GSM_COMMAND;

typedef enum
{
   MIN_FUNCTIONALITI = 0,
   FULL_FUNCTIONALITI = 1,
} GSM_FUNCTIONALITI;

int mc_get(const char* pCmd, u8 m_type, GSM_INFO* pOut, u8 count, u32 second);
void mc_send(const char* pCmd, char* pPrm, uint32_t pause);
RET_INFO GsmModemCmdOn(void);
void GsmModemCmdOff(void);
RET_INFO isOK(const char* pCmd, uint32_t wait);
RET_INFO ModemSettingStart(void);
RET_INFO initGsmModule(void);
RET_INFO configGsmModule(void);
RET_INFO mc(const char* pCmd,
            u32 second,
            int count);   // 1 - имя команды, 2 - таймаут команды, 3 - количество попыток ввода команды
M_INFO SimCardPin(void);
M_INFO SimCardInit(void);
uint64_t readGsmIMEI(char* pIMEI);
int GetSimScidFromGsmModule(char* pSCID);
_Bool GsmModemConnectGprsService(void);   // 0-Attach, 1-Detach from GPRS Service.
RET_INFO SetGsmFunctional(GSM_FUNCTIONALITI eState);
void SetupInternetConnectionProfile(const char* pUser, const char* pPasswd, const char* pApn, _Bool bTypeProf);
int ActivationConnectToGprs(uint8_t usProf);
RET_INFO modem_jamming_detected(uint8_t ucTimeFindJammingDetect);
void gsm_write(const char* pBuf, int size, uint32_t pause);
int gsm_gets(char* pBuf, int size, uint32_t wait);
int gsm_read(char* pBuf, int size, uint32_t wait);
#endif