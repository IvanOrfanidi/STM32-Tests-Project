
#include "rtc.h"
#include "stm32l0xx_hal.h"
#include "main.h"
#include "gpio.h"

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

static void initializeRtcOnly();
static void initializeRtcTimeDate();
static void enableAlarmA();

void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
{
    if(hrtc->Instance == RTC) {
        /* Peripheral clock enable */
        __HAL_RCC_RTC_ENABLE();
    }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* hrtc)
{
    if(hrtc->Instance == RTC) {
        /* Peripheral clock disable */
        __HAL_RCC_RTC_DISABLE();
    }
}

/* RTC init function */
void MX_RTC_Init(void)
{
    initializeRtcOnly();    // Initialize RTC Only

    initializeRtcTimeDate();    // Initialize RTC and set the Time and Date

    // enableAlarmA();               //Enable the Alarm A
    // enableRtcTamper1();           //Enable the RTC Tamper 1
}

/**Initialize RTC Only  */
static void initializeRtcOnly(void)
{
    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv = 255;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    if(HAL_RTC_Init(&hrtc) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }
}

/**Initialize RTC and set the Time and Date  */
static void initializeRtcTimeDate(void)
{
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;

    if(HAL_RTCEx_BKUPRead(&hrtc, DEF_REG_CONFIG_RTC) != 0x32F2)    //Проверка на включение RTC.
    {
        sTime.Hours = 0;
        sTime.Minutes = 0;
        sTime.Seconds = 0;
        sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
        sTime.StoreOperation = RTC_STOREOPERATION_RESET;
        if(HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
            _Error_Handler(__FILE__, __LINE__);
        }

        sDate.WeekDay = CURENT_WEEKDAY;
        sDate.Date = CURENT_DATE;
        sDate.Month = CURENT_MOUNTH;
        sDate.Year = CURENT_YEAR;

        if(HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
            _Error_Handler(__FILE__, __LINE__);
        }

        HAL_RTCEx_BKUPWrite(&hrtc, DEF_REG_CONFIG_RTC, 0x32F2);
    }
}

/**Enable the Alarm A  */
static void enableAlarmA(void)
{
    RTC_AlarmTypeDef sAlarm;
    RTC_TimeTypeDef stTime;
    RTC_DateTypeDef stDate;

    HAL_RTC_GetTime(&hrtc, &stTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &stDate, RTC_FORMAT_BIN);
    Sec2Date(&stTime, &stDate, (Date2Sec(&stTime, &stDate) + TIME_SAVE_ARCHIVE));

    sAlarm.AlarmTime.Hours = stTime.Hours;
    sAlarm.AlarmTime.Minutes = stTime.Minutes;
    sAlarm.AlarmTime.Seconds = stTime.Seconds;
    sAlarm.AlarmTime.SubSeconds = 0;

    extern _Bool __DEBUG__;
    if(__DEBUG__) {
        printf("Next Alarm A: %02d:%02d:%02d\r\n",
            sAlarm.AlarmTime.Hours,
            sAlarm.AlarmTime.Minutes,
            sAlarm.AlarmTime.Seconds);
    }

    sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT_24;
    sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
    sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;
    sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
    sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    sAlarm.AlarmDateWeekDay = 1;
    sAlarm.Alarm = RTC_ALARM_A;
    if(HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }
    __HAL_RTC_ALARM_ENABLE_IT(&hrtc, RTC_IT_ALRA);
}

void Sec2Date(RTC_TimeTypeDef* pDestTime, RTC_DateTypeDef* pDestDate, uint32_t ulSec)
{
    uint32_t dl = 0;
    pDestDate->WeekDay = 0;

    if(ulSec >= 946684800L) {
        ulSec -= 946684800L;    //Дата позже 1 января 2000 года
        pDestDate->Year = 0;
    }
    else {
        pDestDate->Year = 70;    //Дата от 1970 года до 1999 года
    }

    for(dl = 365L; ulSec >= (dl = 86400L * (365L + visocosn(pDestDate->Year))); ulSec -= dl, pDestDate->Year++)
        ;

    const uint8_t mon_len[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    for(pDestDate->Month = 1;
        ulSec >=
        (dl = 86400L * (mon_len[pDestDate->Month] + ((pDestDate->Month == 2) ? visocosn(pDestDate->Year) : 0)));
        ulSec -= dl, pDestDate->Month++)
        ;

    pDestDate->Date = ulSec / (86400L) + 1;
    ulSec %= 86400L;
    pDestTime->Hours = ulSec / 3600L;
    ulSec %= 3600L;
    pDestTime->Minutes = ulSec / 60L;
    ulSec %= 60L;
    pDestTime->Seconds = ulSec;
}

uint32_t Date2Sec(RTC_TimeTypeDef* pDestTime, RTC_DateTypeDef* pDestDate)
{
    uint32_t ulTimeInSec = 0;
    uint32_t tmp = 0;

    if(pDestDate->Year < 70) {
        ulTimeInSec += 946684800L;    //Дата позже 1 января 2000 года
        for(tmp = 0; tmp < pDestDate->Year; ulTimeInSec += 86400L * (365L + visocosn(tmp)), tmp++)
            ;
    }
    else {
        for(tmp = 70; tmp < pDestDate->Year; ulTimeInSec += 86400L * (365L + visocosn(tmp)), tmp++)
            ;
    }

    const uint8_t mon_len[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    for(tmp = 0; tmp < pDestDate->Month;
        ulTimeInSec += (86400L * (mon_len[tmp] + ((tmp == 2) ? visocosn(pDestDate->Year) : 0))), tmp++)
        ;

    ulTimeInSec += (86400L) * (pDestDate->Date - 1);
    ulTimeInSec += 3600L * pDestTime->Hours;
    ulTimeInSec += 60L * pDestTime->Minutes;
    ulTimeInSec += pDestTime->Seconds;

    return ulTimeInSec;
}

/** INTERRUPT *****************************************************************/
/*
Общий обработчик перерываний от RTC.
*/
void RTC_IRQHandler(void)
{
    HAL_NVIC_ClearPendingIRQ(RTC_IRQn);
    HAL_RTC_AlarmIRQHandler(&hrtc);
    HAL_RTCEx_TamperTimeStampIRQHandler(&hrtc);
}
/******************************************************************************/

/* ALARM A */
/*
Обработчик формирования пакетов архива.
*/
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef* hrtc)
{
    extern _Bool __DEBUG__;
    if(__DEBUG__) {
        printf("RTC Alarm A\r\nTimer Data\r\n");
    }
    enableAlarmA();    //Переустанавливаем будильник на час вперед.
}
/******************************************************************************/
