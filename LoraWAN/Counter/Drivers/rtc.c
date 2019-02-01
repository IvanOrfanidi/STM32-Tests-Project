
#include "rtc.h"
#include "stm32l0xx_hal.h"
#include "protocol.h"
#include "main.h"
#include "gpio.h"

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

static void initializeRtcOnly();
static void initializeRtcTimeDate();
static void enableAlarmA();
static void enableAlarmB();
static void enableRtcTamper1();
static void setTamper1State(_Bool);

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

    enableAlarmA();    // Enable the Alarm A

    setTamper1State(0);

    enableRtcTamper1();    // Enable the RTC Tamper 1
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

        HAL_RTCEx_BKUPWrite(&hrtc, DEF_REG_COUNT_DAILY, 0x180000);
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

    extern void saveAlarmA(RTC_AlarmTypeDef * pAlarm);
    saveAlarmA(&sAlarm);

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

/**Enable the Alarm B  */
static void enableAlarmB(void)
{
    RTC_AlarmTypeDef sAlarm;
    RTC_TimeTypeDef stTime;
    RTC_DateTypeDef stDate;

    HAL_RTC_GetTime(&hrtc, &stTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &stDate, RTC_FORMAT_BIN);

    Sec2Date(&stTime, &stDate, (Date2Sec(&stTime, &stDate) + (60 * TIME_TAMPER_REACTION)));
    sAlarm.AlarmTime.Hours = stTime.Hours;
    sAlarm.AlarmTime.Minutes = stTime.Minutes;
    sAlarm.AlarmTime.Seconds = stTime.Seconds;

    sAlarm.AlarmTime.SubSeconds = 0;

    extern _Bool __DEBUG__;
    if(__DEBUG__) {
        printf("Time Alarm Tamper State: %02d:%02d:%02d\r\n",
            sAlarm.AlarmTime.Hours,
            sAlarm.AlarmTime.Minutes,
            sAlarm.AlarmTime.Seconds);
    }

    extern void saveAlarmB(RTC_AlarmTypeDef * pAlarm);
    saveAlarmB(&sAlarm);

    sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
    sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;
    sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
    sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    sAlarm.AlarmDateWeekDay = 0x1;
    sAlarm.Alarm = RTC_ALARM_B;
    if(HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }
    __HAL_RTC_ALARM_ENABLE_IT(&hrtc, RTC_IT_ALRB);
}

/**Enable the RTC Tamper 1 */
static void enableRtcTamper1(void)
{
    RTC_TamperTypeDef sTamper;

    sTamper.Tamper = RTC_TAMPER_1;
    sTamper.Interrupt = RTC_TAMPER1_INTERRUPT;
    sTamper.Trigger = RTC_TAMPERTRIGGER_FALLINGEDGE;
    sTamper.NoErase = RTC_TAMPER_ERASE_BACKUP_DISABLE;
    sTamper.MaskFlag = RTC_TAMPERMASK_FLAG_DISABLE;
    sTamper.Filter = RTC_TAMPERFILTER_4SAMPLE;    // RTC_TAMPERFILTER_DISABLE;
    sTamper.SamplingFrequency = RTC_TAMPERSAMPLINGFREQ_RTCCLK_DIV256;
    sTamper.PrechargeDuration = RTC_TAMPERPRECHARGEDURATION_1RTCCLK;
    sTamper.TamperPullUp = RTC_TAMPER_PULLUP_ENABLE;
    sTamper.TimeStampOnTamperDetection = RTC_TIMESTAMPONTAMPERDETECTION_ENABLE;
    if(HAL_RTCEx_SetTamper_IT(&hrtc, &sTamper) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }
}

/**Disable the RTC Tamper 1 */
void disableRtcTamper1(void)
{
    if(HAL_RTCEx_DeactivateTamper(&hrtc, RTC_TAMPER_1) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }
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

static void setTamper1State(_Bool stat)
{
    HAL_RTCEx_BKUPWrite(&hrtc, DEF_REG_TAMPER_STATE, stat);
}

/*
Получить состояние тампера(по факту значение регистра backup)
Input: no.
Return: состояние тампера (1 - разомкнут, 0 - замкнут)
*/
_Bool getTamper1State(void)
{
    _Bool tamper = (_Bool)HAL_RTCEx_BKUPRead(&hrtc, DEF_REG_TAMPER_STATE);
    extern _Bool __DEBUG__;
    if(__DEBUG__) {
        printf("Tamper State: %d\r\n", tamper);
    }
    return tamper;
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

    saveDataArchive();    //Формируем архив записи в EEPROM.!!!

    /* Сормируем если надо пакет «даннх по расписанию» */
    // получаем счетчик часов
    uint32_t count_daily = HAL_RTCEx_BKUPRead(hrtc, DEF_REG_COUNT_DAILY);
    count_daily &= ~0xFFFFFF00;
    count_daily++;
    // получаем интервал в часах выхода на связь
    uint32_t interval_send = HAL_RTCEx_BKUPRead(hrtc, DEF_REG_COUNT_DAILY);
    interval_send >>= 16;

    extern void saveInterval(uint32_t count, uint32_t interval);
    saveInterval(count_daily, interval_send);

    if(count_daily >= interval_send) {
        count_daily = 0;

        /* сформируем и отправим пакет по расписанию */
        uint8_t data_timer[SIZE_TIMER_PACKET];
        int len = buildDataTimer(data_timer, sizeof(data_timer));
        if(len > 0) {
            /* Send to LoRa data timer */
            // SendDataTimer(data_timer, len);
        }
    }

    interval_send = HAL_RTCEx_BKUPRead(hrtc, DEF_REG_COUNT_DAILY);
    interval_send &= ~0x0000FFFF;
    count_daily |= interval_send;
    HAL_RTCEx_BKUPWrite(hrtc, DEF_REG_COUNT_DAILY, count_daily);
}
/******************************************************************************/

/* ALARM B */
void HAL_RTCEx_AlarmBEventCallback(RTC_HandleTypeDef* hrtc)
{
    __HAL_RTC_ALARM_DISABLE_IT(hrtc, RTC_IT_ALRB);

    extern _Bool __DEBUG__;
    if(__DEBUG__) {
        printf("RTC Alarm B\r\nTamper Data\r\n");
    }

    setTamper1State(0);    // Save tamper FALSE.

    /* сформируем и отправим пакет тампера */
    uint8_t data_tamer[SIZE_TAMPER_PACKET];
    int len = buildDataTamper(data_tamer, sizeof(data_tamer));
    if(len > 0) {
        /* Send to LoRa data tamper state */
        // SendDataTamperState(data_tamer, len);
    }
}

/* TAMPER */
void HAL_RTCEx_Tamper1EventCallback(RTC_HandleTypeDef* hrtc)
{
    extern _Bool __DEBUG__;
    if(__DEBUG__) {
        printf("Tamper Alarm\r\n");
    }
    // disableRtcTamper1();              //Disable Tamper.
    if(!(getTamper1State())) {
        extern _Bool __DEBUG__;
        if(__DEBUG__) {
            printf("Tamper Data\r\n");
        }
        setTamper1State(1);    // Save tamper TRUE.

        uint8_t data_tamer[SIZE_TAMPER_PACKET];
        int len = buildDataTamper(data_tamer, sizeof(data_tamer));
        if(len > 0) {
            /* Send to LoRa data tamper state */
            // SendDataTamperState(data_tamer, len);
        }
    }
    enableAlarmB();    // Enable the Alarm B.
}
