#ifndef _GSM_GPRS_H_
#define _GSM_GPRS_H_

#include "includes.h"
#include "gsm_parser.h"
#include "eeprom.h"

typedef enum
{
   PROF_FIRST_SERVER = 0,
   PROF_SECOND_SERVER = 1,
   PROF_AGPS_SERVER = 2,
   PROF_HTTP_GET_SERVER = 3,
   PROF_HTTP_POST_SERVER = 4,
} PROF_CONNECT_SERVER;

#define TX_READY 1
#define RX_READY 2

#define GET 0
#define POST 1
#define HEAD 2

RET_INFO profile_gprs_connect(_Bool bTypeProf);
RET_INFO ProfileSocketInit(char* pNameSocket, uint8_t nProf);
void profile_socket_http(char* pNameServ, uint8_t nProf);
RET_INFO ProfileLogSocketInit(void);

RET_INFO profile_activate(
   PROF_CONNECT_SERVER nProf,
   const char* pNameServ);   // соединяемся с сервером по gprs, pNameServ - нужен только для SIM800
RET_INFO profile_deactivate(uint8_t nProf);   // закрываем gprs соединение
int mc_siso(uint8_t nProf,
            uint8_t count,
            uint8_t second);   // 1 Профиль(в SIM800 еще и команда чтения ftp), 2 - колво попыток, 3 - задержка в сек.
int socket_send(u8 nProf, char* data_buf, int data_size, u8* mask_ready);
int socket_read(u8 nProf, char* pBuf, int size);
RET_INFO wait_rx_ready(uint8_t nProf, uint16_t sec);

#endif