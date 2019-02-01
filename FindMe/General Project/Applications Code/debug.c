

#include <stdarg.h>

#include "includes.h"
#include "debug.h"

char d_buf[DBG_TX_BUFFER_SIZE];

/* Структура вывода отладочных сообщений */
typedef __packed struct
{
    _Bool bGlobal_Debug_USART;    //Вкл/Выкл всю отладку.
    _Bool bGSM_Debug_USART;
    _Bool bGPS_Debug_USART;
    _Bool bACCEL_Debug_USART;
    _Bool bTIME_Msg_Debug_USART;
} TUART_DEBUG;
TUART_DEBUG stDebugUSART;

const char* const gsm_rw[] = {
    "OK",                // 00
    "CONNECT",           // 01
    "RING",              // 02
    "NO CARRIER",        // 03
    "ERROR",             // 04
    "NO DIALTONE",       // 05
    "BUSY",              // 06
    "NO ANSWER",         // 07
    "PROCEEDING",        // 08
    "connect <text>",    // 09
    "TIMEOUT"            // 0A
};

void printDeviceTime(void)
{
    char msg_time_dev[20];
    uint32_t sec = GetWakingTime();
    RTC_t stDate;
    Sec2Date(&stDate, sec);
    sprintf(msg_time_dev,
        "%02d/%02d/%02d %02d:%02d:%02d\r\n",
        stDate.mday,
        stDate.month,
        stDate.year,
        stDate.hour,
        stDate.min,
        stDate.sec);

    DPS("D_DEVICE WAKING DATE: ");
    DPS(msg_time_dev);
}

void setDebugAll(_Bool dbg)
{
    stDebugUSART.bGlobal_Debug_USART = dbg;
    stDebugUSART.bACCEL_Debug_USART = dbg;
    stDebugUSART.bGPS_Debug_USART = dbg;
    stDebugUSART.bGSM_Debug_USART = dbg;
    stDebugUSART.bTIME_Msg_Debug_USART = dbg;
}

void setDebugGps(_Bool dbg)
{
    stDebugUSART.bGPS_Debug_USART = dbg;
}

void setDebugAccel(_Bool dbg)
{
    stDebugUSART.bACCEL_Debug_USART = dbg;
}

void setDebugGsm(_Bool dbg)
{
    stDebugUSART.bGSM_Debug_USART = dbg;
}

void setDebug(_Bool dbg)
{
    stDebugUSART.bGlobal_Debug_USART = dbg;
}

_Bool getDebug(void)
{
    return (stDebugUSART.bGlobal_Debug_USART == 1);
}

void GPS_DPD(char* pData_Usart, uint16_t Len)
{
    if(!(stDebugUSART.bGPS_Debug_USART)) {
        return;
    }

    DPD((char*)pData_Usart, Len);
}

void GPS_DPS(char* pData_Usart)
{
    if(!(stDebugUSART.bGPS_Debug_USART)) {
        return;
    }
    DPS((char*)pData_Usart);
}

void ACCEL_DPD(char* pData_Usart, uint16_t Len)
{
    if(Len > DBG_TX_BUFFER_SIZE)
        return;
    if(!(stDebugUSART.bACCEL_Debug_USART)) {
        return;
    }

    DPD((char*)pData_Usart, Len);
}

void GSM_DPD(char* pData_Usart, uint16_t Len)
{
    if(Len > DBG_TX_BUFFER_SIZE)
        return;
    if(!(stDebugUSART.bGSM_Debug_USART)) {
        return;
    }

    DPD((char*)pData_Usart, Len);
}

void DS_GSM(const char* msg, const char* pData_Usart)
{
    if(!(stDebugUSART.bGSM_Debug_USART)) {
        return;
    }

    uint16_t Len = 0;

    d_buf[0] = 0;
    strcpy(d_buf, msg);
    strcat(d_buf, pData_Usart);

    while(d_buf[Len] != 0) {
        if((d_buf[Len] == '\r') || (d_buf[Len] == '\n'))
            d_buf[Len] = 0x20;

        Len++;
    }

    strcat(d_buf, "\r\n");

    DPS(d_buf);
}

void DPD(const char* pData_Usart, uint16_t Len)
{
    if(Len > DBG_TX_BUFFER_SIZE)
        return;
    if(!(stDebugUSART.bGlobal_Debug_USART)) {
        return;
    }
    if(osKernelRunning()) {
        xSemaphoreTake(sBinSemUSART, portMAX_DELAY);
    }
    USART_Write(UART_DBG, pData_Usart, Len);
    if(osKernelRunning()) {
        xSemaphoreGive(sBinSemUSART);
    }
}

void DPC(char Data_Usart)
{
    if(!(stDebugUSART.bGlobal_Debug_USART)) {
        return;
    }

    DPD(&Data_Usart, 1);
}

void DPS(const char* pData_Usart)
{
    uint16_t Len;
    if(!(stDebugUSART.bGlobal_Debug_USART)) {
        return;
    }

    if(stDebugUSART.bTIME_Msg_Debug_USART /*&& (pData_Usart[0] == '\r')*/) {
        //получим текущее время
        RTC_t stDate;
        getSystemDate(&stDate);
        char TimeMsg[20];
        sprintf(TimeMsg, "\r\n%02d/", stDate.mday);
        sprintf(TimeMsg + strlen(TimeMsg), "%02d/", stDate.month);
        sprintf(TimeMsg + strlen(TimeMsg), "%02d ", stDate.year);
        sprintf(TimeMsg + strlen(TimeMsg), "%02d:", stDate.hour);
        sprintf(TimeMsg + strlen(TimeMsg), "%02d:", stDate.min);
        sprintf(TimeMsg + strlen(TimeMsg), "%02d> ", stDate.sec);

        if(strlen(pData_Usart) > 3) {
            if(osKernelRunning()) {
                xSemaphoreTake(sBinSemUSART, portMAX_DELAY);
            }
            USART_Write(UART_DBG, TimeMsg, strlen(TimeMsg));
            if(osKernelRunning()) {
                xSemaphoreGive(sBinSemUSART);
            }
        }
    }

    Len = strlen(pData_Usart);
    if(Len > DBG_TX_BUFFER_SIZE) {
        return;
    }

    if(osKernelRunning()) {
        xSemaphoreTake(sBinSemUSART, portMAX_DELAY);
    }
    USART_Write(UART_DBG, pData_Usart, Len);
    if(osKernelRunning()) {
        xSemaphoreGive(sBinSemUSART);
    }
}

void debug_printf(const char* fmt_ptr, ...)
{
    if((!(stDebugUSART.bGlobal_Debug_USART) || (!(stDebugUSART.bGSM_Debug_USART)))) {
        return;
    }

    va_list ap;

    va_start(ap, fmt_ptr);
    vsprintf(d_buf, fmt_ptr, ap);
    va_end(ap);

    DPS(d_buf);
}

void GSM_DC(const char* pCmd, char res)
{
    if(!(stDebugUSART.bGSM_Debug_USART)) {
        return;
    }

    GSM_WP(pCmd, 0, res);
}

void GSM_WP(const char* pCmd, const char* pPmd, char res)
{
    if(!(stDebugUSART.bGSM_Debug_USART)) {
        return;
    }

    char n;
    char ch = 0;

    n = ((res < '0' || res > '9') ? 10 : res - '0');

    switch(n) {
        case 0:
            ch = '+';
            break;
        case 4:
            ch = '-';
            break;
        case 10:
            ch = 't';
            break;
    }

    if(ch != 0) {
        debug_printf("[%c] GSM: %s", ch, pCmd);
        if(pPmd != 0)
            DPS(pPmd);
    }
    else {
        DPS("GSM: ");
        DPS(pCmd);
        if(pPmd != 0) {
            DPS(pPmd);
        }
        DPC(' ');
        DPS(gsm_rw[n]);
    }

    DPC('\r');
}

#pragma optimize = none
void EmulatorHardFault(void)
{
    volatile uint8_t* p = (uint8_t*)0x8FFFFFF;
    volatile uint8_t a = *p;
    if(a) {
        a = 0;
    }
}