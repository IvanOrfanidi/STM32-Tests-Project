
#ifndef _GSM_HTTP_H_
#define _GSM_HTTP_H_

#include "includes.h"
#include "eeprom.h"
#include "sram.h"

#define TIMEOUT_HTTP_CONNECT 30   //Ждем подключения
#define MAX_ERR_CONNECT_HTTP 5
#define MAX_ERR_DOWNLOAD_PAGE 4
#define MAX_ERR_DOWN_SESSION 10

#define MAX_ERR_DOWN_FIRM_HTTP 5

RET_INFO profile_http_activate(PROF_CONNECT_SERVER usProf, const char* pNameServ);
uint8_t profile_http_read(PROF_CONNECT_SERVER usProf);
uint8_t check_firmware(void);

#endif