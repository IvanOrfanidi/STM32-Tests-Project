
#include "includes.h"
#include "rtc.h"

#include <stdio.h>

#define TIMEOUT_START_RTC_CLOCK 100000

void getSystemDate(RTC_t* date)
{
    RTC_TimeTypeDef RTC_TimeStructure;
    RTC_DateTypeDef RTC_DateStructure;

    date->wday = 0;

    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);

    date->hour = RTC_TimeStructure.RTC_Hours;
    date->min = RTC_TimeStructure.RTC_Minutes;
    date->sec = RTC_TimeStructure.RTC_Seconds;
    date->h12 = RTC_TimeStructure.RTC_H12;

    RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);

    date->month = RTC_DateStructure.RTC_Month;
    date->mday = RTC_DateStructure.RTC_Date;
    date->year = RTC_DateStructure.RTC_Year;
}

int SetAlarmTime(const RTC_t* date)
{
    RTC_AlarmTypeDef RTC_AlarmStructure;

    // enable rtc write
    PWR_RTCAccessCmd(ENABLE);
    // Disable the Alarm B
    RTC_AlarmCmd(RTC_Alarm_B, DISABLE);

    // Disable the Alarm A
    if(RTC_AlarmCmd(RTC_Alarm_A, DISABLE) == ERROR) {
        return -1;
    }

    RTC_AlarmStructure.RTC_AlarmTime.RTC_H12 = date->h12;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours = date->hour;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = date->min;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = date->sec;

    RTC_AlarmStructure.RTC_AlarmDateWeekDay = date->mday;
    RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;
    RTC_AlarmStructure.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;
    RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure);

    // Enable RTC Alarm A Interrupt
    RTC_ITConfig(RTC_IT_ALRA, ENABLE);

    // Enable the Alarm A
    if(RTC_AlarmCmd(RTC_Alarm_A, ENABLE) == ERROR) {
        return -1;
    }

    // RTC_OutputConfig(RTC_Output_AlarmA, RTC_OutputPolarity_High);
    RTC_ITConfig(RTC_IT_ALRA, ENABLE);
    RTC_AlarmCmd(RTC_Alarm_A, ENABLE);

    RTC_ITConfig(RTC_IT_ALRB, DISABLE);
    RTC_AlarmCmd(RTC_Alarm_B, DISABLE);

    RTC_ClearFlag(RTC_FLAG_ALRAF);    // Clear RTC Alarm A Flag
    RTC_ClearFlag(RTC_FLAG_ALRBF);    // Clear RTC Alarm B Flag
    return 0;
}

void ResetAlarmTime(void)
{
    // enable rtc write
    PWR_RTCAccessCmd(ENABLE);

    // Disable the Alarm A
    RTC_AlarmCmd(RTC_Alarm_A, DISABLE);

    // Disable the Alarm B
    RTC_AlarmCmd(RTC_Alarm_B, DISABLE);

    // Disable RTC Alarm A Interrupt
    RTC_ITConfig(RTC_IT_ALRA, DISABLE);

    // Disable RTC Alarm B Interrupt
    RTC_ITConfig(RTC_IT_ALRB, DISABLE);

    // Clear RTC Alarm A Flag
    RTC_ClearFlag(RTC_FLAG_ALRAF);

    // Clear RTC Alarm B Flag
    RTC_ClearFlag(RTC_FLAG_ALRBF);
}

void GetAlarmTime(RTC_t* date)
{
    RTC_AlarmTypeDef RTC_AlarmStructure;

    RTC_GetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmStructure);

    date->hour = RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours;
    date->min = RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes;
    date->sec = RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds;
    date->h12 = RTC_AlarmStructure.RTC_AlarmTime.RTC_H12;
}

void setSystemDate(const RTC_t* date)
{
    RTC_TimeTypeDef RTC_TimeStructure;
    RTC_DateTypeDef RTC_DateStructure;

    // Set the Time
    RTC_TimeStructure.RTC_Hours = date->hour;
    RTC_TimeStructure.RTC_Minutes = date->min;
    RTC_TimeStructure.RTC_Seconds = date->sec;
    RTC_TimeStructure.RTC_H12 = RTC_H12_AM;

    // Set the Date
    RTC_DateStructure.RTC_Month = date->month;
    RTC_DateStructure.RTC_Date = date->mday;
    RTC_DateStructure.RTC_Year = date->year;
    RTC_DateStructure.RTC_WeekDay = date->wday;

    // enable rtc write
    PWR_RTCAccessCmd(ENABLE);
    RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);
    RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);
}

void resetSystemDateTime(void)
{
    RTC_t stDefDateRTC;
    Sec2Date(&stDefDateRTC, DEF_TIME_DATA);    // 01 Jan 2017 23:50:00 GMT
    setSystemDate(&stDefDateRTC);
    SetAlarmTime(&stDefDateRTC);
}

void RTC_Configuration_HSE(void)
{
    RTC_InitTypeDef RTC_InitStructure;
    // SaveDataRTC();

    /* Enable the LSI OSC */
    RCC_LSICmd(ENABLE);

    /* Enable the PWR clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    /* Allow access to RTC */
    PWR_RTCAccessCmd(ENABLE);

    // Clear WakeIp flag
    PWR_ClearFlag(PWR_FLAG_WU);

    // Check if the StandBy flag is set
    if(PWR_GetFlagStatus(PWR_FLAG_SB) != RESET) {
        // Clear StandBy flag
        PWR_ClearFlag(PWR_FLAG_SB);
    }

    /* Allow access to RTC */
    PWR_RTCAccessCmd(ENABLE);

    // Reset Backup Domain
    RCC_RTCResetCmd(ENABLE);
    RCC_RTCResetCmd(DISABLE);

    RCC_HSEConfig(RCC_HSE_ON);

    /* Wait till LSE is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET) {
        IWDG_ReloadCounter();
    }

    /* Enable the RTC Clock */
    RCC_RTCCLKCmd(ENABLE);

    /* Select the RTC Clock Source */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div4);

    // Calender Configuartion
    RTC_InitStructure.RTC_AsynchPrediv = 125;    // Clock 1 MHz
    RTC_InitStructure.RTC_SynchPrediv = 8000;
    RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
    RTC_Init(&RTC_InitStructure);

    /* Wait for RTC APB registers synchronisation */
    RTC_WaitForSynchro();

    /* Enable the RTC Clock */
    RCC_RTCCLKCmd(ENABLE);

    PWR_RTCAccessCmd(DISABLE);

    /* Wait for RTC APB registers synchronisation */
    RTC_WaitForSynchro();

    // Clear RTC Alarm Flag
    RTC_ClearFlag(RTC_FLAG_ALRAF);

    // ReadDataRTC();
}

void RTC_Configuration_LSI(void)
{
    RTC_InitTypeDef RTC_InitStructure;
    // SaveDataRTC();
    /* Enable the LSI OSC */
    RCC_LSICmd(ENABLE);

    /* Enable the PWR clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    /* Allow access to RTC */
    PWR_RTCAccessCmd(ENABLE);

    // Clear WakeIp flag
    PWR_ClearFlag(PWR_FLAG_WU);

    // Check if the StandBy flag is set
    if(PWR_GetFlagStatus(PWR_FLAG_SB) != RESET) {
        // Clear StandBy flag
        PWR_ClearFlag(PWR_FLAG_SB);
    }

    /* Allow access to RTC */
    PWR_RTCAccessCmd(ENABLE);

    // Reset Backup Domain
    RCC_RTCResetCmd(ENABLE);
    RCC_RTCResetCmd(DISABLE);

    RCC_LSICmd(ENABLE);

    /* Wait till LSI is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET) {
        IWDG_ReloadCounter();
    }

    /* Enable the RTC Clock */
    RCC_RTCCLKCmd(ENABLE);

    /* Select the RTC Clock Source */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

    // Calender Configuartion
    RTC_InitStructure.RTC_AsynchPrediv = 0x7F;    // Clock 32767Hz
    RTC_InitStructure.RTC_SynchPrediv = 0xFF;
    RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
    RTC_Init(&RTC_InitStructure);

    /* Wait for RTC APB registers synchronisation */
    RTC_WaitForSynchro();

    /* Enable the RTC Clock */
    RCC_RTCCLKCmd(ENABLE);

    PWR_RTCAccessCmd(DISABLE);

    /* Wait for RTC APB registers synchronisation */
    RTC_WaitForSynchro();

    // Clear RTC Alarm Flag
    RTC_ClearFlag(RTC_FLAG_ALRAF);

    // ReadDataRTC();
}

int RTC_Configuration(void)
{
    RTC_InitTypeDef RTC_InitStructure;
    __IO uint32_t StartUpCounter = 0, HSEStatus = 0;

    /* Enable the LSI OSC */
    RCC_LSICmd(ENABLE);

    /* Enable the PWR clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    /* Allow access to RTC */
    PWR_RTCAccessCmd(ENABLE);

    // Clear WakeIp flag
    PWR_ClearFlag(PWR_FLAG_WU);

    // Check if the StandBy flag is set
    if(PWR_GetFlagStatus(PWR_FLAG_SB) != RESET) {
        // Clear StandBy flag
        PWR_ClearFlag(PWR_FLAG_SB);
    }

    // Reset Backup Domain
    // RCC_RTCResetCmd(ENABLE);
    // RCC_RTCResetCmd(DISABLE);

    if(!(RCC->CSR & RCC_CSR_RTCEN)) {
        /* Allow access to RTC */
        PWR_RTCAccessCmd(ENABLE);

        // Reset Backup Domain
        RCC_RTCResetCmd(ENABLE);
        RCC_RTCResetCmd(DISABLE);

#ifdef RTC_CLOCK_SOURCE_LSI /* LSI used as RTC source clock*/
        /* The RTC Clock may varies due to LSI frequency dispersion. */
        /* Enable the LSI OSC */
        RCC_LSICmd(ENABLE);

        /* Wait till LSI is ready */
        while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET) {
            IWDG_ReloadCounter();
        }

        /* Enable the RTC Clock */
        RCC_RTCCLKCmd(ENABLE);

        /* Select the RTC Clock Source */
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

        // Calender Configuartion
        RTC_InitStructure.RTC_AsynchPrediv = 0x7F;    // Clock 32767Hz
        RTC_InitStructure.RTC_SynchPrediv = 0xFF;
        RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
        RTC_Init(&RTC_InitStructure);

        /* Wait for RTC APB registers synchronisation */
        RTC_WaitForSynchro();

#endif

#ifdef RTC_CLOCK_SOURCE_LSE /* LSE used as RTC source clock */

        /* Enable the LSE OSC */
        RCC_LSEConfig(RCC_LSE_ON);

        /* Wait till LSE is ready */
        while((RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && (StartUpCounter <= TIMEOUT_START_RTC_CLOCK)) {
            IWDG_ReloadCounter();
            StartUpCounter++;
        }

        if(StartUpCounter >= TIMEOUT_START_RTC_CLOCK) {
            // RTC_Configuration_LSI();
            // return -1;
        }

        /* Enable the RTC Clock */
        RCC_RTCCLKCmd(ENABLE);

        /* Select the RTC Clock Source */
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

        // Calender Configuartion
        RTC_InitStructure.RTC_AsynchPrediv = 0x7F;    // Clock 32767Hz
        RTC_InitStructure.RTC_SynchPrediv = 0xFF;
        RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
        RTC_Init(&RTC_InitStructure);

        /* Wait for RTC APB registers synchronisation */
        RTC_WaitForSynchro();
#endif /* RTC_CLOCK_SOURCE_LSI */

#ifdef RTC_CLOCK_SOURCE_HSE /* HSE used as RTC source clock */

        RCC_HSEConfig(RCC_HSE_ON);
        /* Wait till LSE is ready */
        while((RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET) && (StartUpCounter <= TIMEOUT_START_RTC_CLOCK)) {
            IWDG_ReloadCounter();
            StartUpCounter++;
        }

        if(StartUpCounter >= TIMEOUT_START_RTC_CLOCK) {
            // RTC_Configuration_LSI();
            // return -1;
        }
        /* Enable the RTC Clock */
        RCC_RTCCLKCmd(ENABLE);

        /* Select the RTC Clock Source */
        RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div4);

        // Calender Configuartion
        RTC_InitStructure.RTC_AsynchPrediv = 125;    // Clock 1 MHz
        RTC_InitStructure.RTC_SynchPrediv = 8000;
        RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
        RTC_Init(&RTC_InitStructure);

        /* Wait for RTC APB registers synchronisation */
        RTC_WaitForSynchro();

#endif    // RTC_CLOCK_SOURCE_HSE

        /* Enable the RTC Clock */
        RCC_RTCCLKCmd(ENABLE);

        PWR_RTCAccessCmd(DISABLE);

        u8 rtc_write_protect;
        //сохраняем текущий write protect для регистров backup
        if(PWR->CR & PWR_CR_DBP) {
            rtc_write_protect = 0;
        }
        else {
            rtc_write_protect = 1;
        }

        if(rtc_write_protect) {
            PWR_RTCAccessCmd(ENABLE);
        }

        if(rtc_write_protect) {
            PWR_RTCAccessCmd(DISABLE);
        }

        /* Wait for RTC APB registers synchronisation */
        RTC_WaitForSynchro();

        // Clear RTC Alarm Flag
        RTC_ClearFlag(RTC_FLAG_ALRAF);
        return 1;
    }

    /* Wait for RTC APB registers synchronisation */
    RTC_WaitForSynchro();

    // Clear RTC Alarm Flag
    RTC_ClearFlag(RTC_FLAG_ALRAF);

    return 0;
}

void InitAlarm(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    // Enable the EXTI Line17 Interrupt
    EXTI_ClearITPendingBit(EXTI_Line17);
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Line = EXTI_Line17;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // Enable the RTC Alarm Interrupt
    NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*
Получаем системное время девайса.
@param None
@retval время в формате Unix timestamp
*/
uint32_t time(void)
{
    RTC_t DateRTC;
    getSystemDate(&DateRTC);
    return Date2Sec(&DateRTC);
}