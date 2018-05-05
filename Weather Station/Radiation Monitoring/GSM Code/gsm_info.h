#ifndef _GSM_INFO
#define _GSM_INFO

#include "includes.h"
#include "gsm_parser.h"

typedef struct
{
   // csq
   u8 rssi;
   u8 ber;

   // creg
   u8 urcMode;
   u8 regStatus;
   u8 netLac;
   u8 netCellId;

   // cops
   u8 mode;
   u8 format;
   u32 opName;

} GSM_PARAM;

int get_opInfo(const char* cmd, GSM_INFO* gsm, int second);
int lbsInfo2buffer(GSM_INFO* gsm, uint32_t time, char* pOut, int iOffset);
int gsm_get_location(GSM_INFO* gsm, int second);   //Получение gsm станций для SIM800
int gsm_getTemperature(GSM_INFO* pDataGsmInfo);

#endif