#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "includes.h"
#include "eeprom.h"

#ifndef OUT_DEBUG_DATA_SER
#define OUT_DEBUG_DATA_SER 0
#endif

#define DP_GSM(format, ...) debug_printf(format, ##__VA_ARGS__)

void debug_printf(const char* fmt_ptr, ...);
void GSM_WP(const char* pCmd, const char* pPmd, char res);
void GSM_DC(const char* pCmd, char res);
void GPS_DPD(char* pData_Usart, uint16_t Len);
void GPS_DPS(char* pData_Usart);
void ACCEL_DPD(char* pData_Usart, uint16_t Len);
void GSM_DPD(char* pData_Usart, uint16_t Len);
void DPD(const char* pData_Usart, uint16_t Len);
void DPS(const char* pData_Usart);
void DPC(char Data_Usart);
void DS_GSM(const char* msg, const char* pData_Usart);

_Bool getDebug(void);
void setDebug(_Bool dbg);
void setDebugAll(_Bool dbg);
void setDebugGps(_Bool dbg);
void setDebugGsm(_Bool dbg);
void setDebugAccel(_Bool dbg);

void printDeviceTime(void);
void MsgDeviceStatusReset(void);
void EmulatorHardFault(void);

#ifdef __cplusplus
}
#endif

#endif