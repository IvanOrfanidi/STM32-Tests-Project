#ifndef RADMON_H_
#define RADMON_H_

#include "includes.h"

#define DEBUG_OUTPUT_USART
//#define DEBUG_OUTPUT_USB

#define DP_DBG(format, ...) debug_printf(format, ##__VA_ARGS__)

void debug_printf(const char* fmt_ptr, ...);
void GSM_WP(const char* pCmd, const char* pPmd, char res);
void GSM_DC(const char* pCmd, char res);
void DPD(const char* pData_Usart, uint16_t Len);
void DPS(const char* pData_Usart);
void DPC(char Data_Usart);
void DS_GSM(const char* msg, const char* pData_Usart);
void GSM_DPD(char* pData_Usart, uint16_t Len);

void vRadMonTask(void* pvParameters);
void vOnewireTask(void* pvParameters);

#endif