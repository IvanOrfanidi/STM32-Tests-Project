#ifndef _GSM_GPRS_H_
#define _GSM_GPRS_H_

#include "includes.h"
#include "gsm_parser.h"

typedef enum
{
   PROF_FIRST_SERVER = 0,
   PROF_SECOND_SERVER = 1,
   PROF_FTP_SERVER = 2,
   PROF_AGPS_SERVER = 3,
   PROF_HTTP_GET_SERVER = 4,
   PROF_HTTP_POST_SERVER = 5,
} PROF_CONNECT_SERVER;

#define TX_READY 1
#define RX_READY 2

#define GET 0
#define POST 1
#define HEAD 2

RET_INFO profile_gprs_connect(_Bool bTypeProf);
RET_INFO ProfileSocketInit(char* pNameSocket, u8 nProf);
int profile_socket(char* pNameServ, u8 nProf);
void profile_socket_http(char* pNameServ, u8 nProf);
RET_INFO ProfileLogSocketInit(void);

RET_INFO profile_activate(
   PROF_CONNECT_SERVER nProf,
   const char* pNameServ);   // соединяемся с сервером по gprs, pNameServ - нужен только для SIM800
RET_INFO profile_deactivate(u8 nProf);   // закрываем gprs соединение
int mc_siso(u8 nProf,
            uint8_t count,
            uint8_t second);   // 1 Профиль(в SIM800 еще и команда чтения ftp), 2 - колво попыток, 3 - задержка в сек.
int socket_send(u8 nProf, char* data_buf, int data_size, u8* mask_ready);
int socket_read(u8 nProf, char* pBuf, int size);
RET_INFO wait_rx_ready(uint8_t nProf, uint16_t sec);

#endif