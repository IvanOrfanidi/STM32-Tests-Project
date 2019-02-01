
#include "debug.h"
#include "includes.h"

#define TIME_SLEEP_DEVICE 5 * 60

void vDebugTask(void* pvParameters)
{
    uint32_t uiTimeRtc;
    DS3231_date_TypeDef stDateDsRAW;
    RTC_t stDateRTC;
    char strMsgDebug[64];

    InitUSART(UART_DBG, DBG_BAUDRATE);
    InitDMA(UART_DBG);
    strcpy(strMsgDebug, "-=RUN APPLICATION=-\r\n");
    USART_Write(UART_DBG, strMsgDebug, strlen(strMsgDebug));

    InitI2C();
    osDelay(SLEEP_MS_100);
    ResetAlarmDs3231();
    osDelay(SLEEP_MS_100);
    InitDs3231();

    DS3231_ReadDateRAW(&stDateDsRAW);
    ConvertDsToRtc(&stDateDsRAW, &stDateRTC);
    uiTimeRtc = Date2Sec(&stDateRTC);
#define MIN_TIME_RESET 1478378000
    if(uiTimeRtc < MIN_TIME_RESET) {    //—читаем что врем€ сбросилось
        Sec2Date(&stDateRTC, MIN_TIME_RESET);
        ConvertRtcToDs(&stDateRTC, &stDateDsRAW);
        DS3231_WriteDateRAW(&stDateDsRAW);
    }

    osDelay(SLEEP_MS_100);

    while(1) {
        DS3231_ReadDateRAW(&stDateDsRAW);
        ConvertDsToRtc(&stDateDsRAW, &stDateRTC);
        sprintf(strMsgDebug, "%02d:%02d:%02d\r\n", stDateRTC.hour, stDateRTC.min, stDateRTC.sec);
        USART_Write(UART_DBG, strMsgDebug, strlen(strMsgDebug));
        osDelay(1000);

        /* SLEEP DOWN */
#if 1
        ResetAlarmDs3231();
        DS3231_ReadDateRAW(&stDateDsRAW);
        ConvertDsToRtc(&stDateDsRAW, &stDateRTC);
        uiTimeRtc = Date2Sec(&stDateRTC) + TIME_SLEEP_DEVICE;
        Sec2Date(&stDateRTC, uiTimeRtc);
        ConvertRtcToDs(&stDateRTC, &stDateDsRAW);
        DS3231_WriteAlarm1RAW(&stDateDsRAW);

        sprintf(strMsgDebug, "-=SLEEP DEVICE=-\r\n");
        USART_Write(UART_DBG, strMsgDebug, strlen(strMsgDebug));
        osDelay(SLEEP_MS_1000);

        /* Enable WKUP pin */
        PWR_WakeUpPinCmd(ENABLE);

        /* Allow access to BKP Domain */
        PWR_BackupAccessCmd(ENABLE);

        PWR_EnterSTANDBYMode();    //  -_-zZ
#endif

        if(GET_INT_SQW_RTC) {
            sprintf(strMsgDebug, "-=DEVICE ALARM=-\r\n");
            USART_Write(UART_DBG, strMsgDebug, strlen(strMsgDebug));
            ResetAlarmDs3231();
            DS3231_ReadDateRAW(&stDateDsRAW);
            ConvertDsToRtc(&stDateDsRAW, &stDateRTC);
            uiTimeRtc = Date2Sec(&stDateRTC) + 10;
            Sec2Date(&stDateRTC, uiTimeRtc);
            ConvertRtcToDs(&stDateRTC, &stDateDsRAW);
            DS3231_WriteAlarm1RAW(&stDateDsRAW);
        }
    }
}
