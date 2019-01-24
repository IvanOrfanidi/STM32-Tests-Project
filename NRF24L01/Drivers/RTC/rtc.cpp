/**
 ******************************************************************************
 * @file    rtc.cpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    28-March-2018
 * @brief   This file provides all the RTC firmware method.
 ******************************************************************************
 * @attention
 *
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "rtc.hpp"
#include "stm32f10x_bkp.h"
#include "stm32f10x_rtc.h"


#define visocosn(Year) ((Year % 4) ? 0 : 1)


const uint8_t mon_len[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };



/**
 * @bref Initialisation RTC
 * @param rtc - Data
 */
void Rtc::Init(const RTC_t* const rtc)
{
    // Init RTC
    if(InitRtc()) {
        // Set current time
        if(nullptr == rtc) {
            SetTime(RTC_t::DEF_TIME);    // here: 1st January 2010 00:00:00
        }
        else {
            SetTime(rtc);
        }
    }
}


/**
 * @bref Initialisation RTC
 */
bool Rtc::InitRtc()
{
    /* Enable PWR and BKP clocks */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);
    
    /* Enable the LSI OSC */
    RCC_LSICmd(ENABLE);

    /* Wait till LSI is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

    if(BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
    {
        /* Backup data register value is not correct or not yet programmed (when
           the first time the program is executed) */

        /* Allow access to BKP Domain */
        PWR_BackupAccessCmd(ENABLE);

        /* Reset Backup Domain */
        BKP_DeInit();

        /* Enable LSE */
        RCC_LSEConfig(RCC_LSE_ON);

        /* Wait till LSE is ready */
        uint32_t Timeout = 100000;
        while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
        {
            Timeout--;
            if(!(Timeout))
                break;
        }

        if(Timeout) {   // LSE - OK.
            
            /* Select LSE as RTC Clock Source */
            RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);    // LSE = 32.768 KHz

            /* Enable RTC Clock */
            RCC_RTCCLKCmd(ENABLE);

            /* Wait for RTC registers synchronization */
            RTC_WaitForSynchro();

            /* Wait until last write operation on RTC registers has finished */
            RTC_WaitForLastTask();

            /* Set RTC prescaler: set RTC period to 1sec */
            RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768
                                          KHz)/(32767+1) */
        }
        else {   // LSE - NO.
            
            uint32_t count = 10000;

            // Enable the LSI OSC
            RCC_LSICmd(ENABLE);

            // Wait till LSI is ready
            while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET && (--count) > 0)
            {
                __NOP();
                __NOP();
                __NOP();
                __NOP();
            }

            // Select the RTC Clock Source
            RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

            // Enable RTC Clock
            RCC_RTCCLKCmd(ENABLE);

            // Wait for RTC registers synchronization
            RTC_WaitForSynchro();

            // Wait until last write operation on RTC registers has finished
            RTC_WaitForLastTask();

            // Set RTC prescaler: set RTC period to 1sec
            RTC_SetPrescaler(40000);
        }

        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();

        BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);

        /* Lock access to BKP Domain */
        PWR_BackupAccessCmd(DISABLE);
        
        return true; 
    }
    
    return false; 
}

        
/**
 * @bref Enable the LSI OSC
 * @retval true - success, false - fail
 */
bool Rtc::LseEnable()
{
    uint32_t count = 0xFFFFFFFF;

    // LSE Enable
    RCC_LSEConfig(RCC_LSE_ON);

    // Wait till LSE is ready
    while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET && (--count) > 0) {
        __NOP();
        __NOP();
        __NOP();
        __NOP();
    }
    
    if(0 == count) {
        return false;
    }

    // LCD Clock Source Selection
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

    /* Enable RTC Clock */
    RCC_RTCCLKCmd(ENABLE);

    /* Wait for RTC registers synchronization */
    RTC_WaitForSynchro();

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

    /* Set RTC prescaler: set RTC period to 1sec */
    RTC_SetPrescaler(32767);    // RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1)

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

    /* Wait for RTC registers synchronization */
    RTC_WaitForSynchro();

    return true;
}


/**
 * @bref Enable the LSI OSC
 * @retval true - success, false - fail
 */
bool Rtc::LsiEnable()
{
    uint32_t count = 10000;

    // Enable the LSI OSC
    RCC_LSICmd(ENABLE);

    // Wait till LSI is ready
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET && (--count) > 0) {
        __NOP();
        __NOP();
        __NOP();
        __NOP();
    }
    
    if(0 == count) {
        return false;
    }

    // Select the RTC Clock Source
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

    /* Enable RTC Clock */
    RCC_RTCCLKCmd(ENABLE);

    /* Wait for RTC registers synchronization */
    RTC_WaitForSynchro();

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

    /* Set RTC prescaler: set RTC period to 1sec */
    RTC_SetPrescaler(40000);    // RTC period = RTCCLK/RTC_PR = (~40 KHz)

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

    /* Wait for RTC registers synchronization */
    RTC_WaitForSynchro();

    return true;
}


/**
 * @bref Set time
 * @param rtc - Time
 */
void Rtc::SetTime(time_t rtc)
{
    const uint32_t cnt = (uint32_t)rtc;
    
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
    RTC_SetCounter(cnt);

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
}


/**
 * @bref Set time
 * @param [in] rtc - Time
 */
void Rtc::SetTime(const RTC_t* const rtc)
{
    uint32_t cnt = Date2Sec(rtc);
    
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
    RTC_SetCounter(cnt);

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
}


/**
 * @bref Geting current time
 * @retval Current time
 */
time_t Rtc::GetTime()
{
    return (time_t)RTC_GetCounter();
}


/**
 * @brief Geting current time
 * @param [out] rtc - Current time
 */
void Rtc::GetTime(RTC_t* rtc)
{
    Sec2Date(rtc, RTC_GetCounter());
}


/**
 * @brief Convert sec to date
 * @param [out] rtc - date
 * @param [in] sec - time
 */
void Rtc::Sec2Date(RTC_t* rtc, time_t sec)
{
    if(sec >= 946684800L) {
        sec -= 946684800L;    //Дата позже 1 января 2000 года
        rtc->Year = 0;
    }
    else {
        rtc->Year = 70;    //Дата от 1970 года до 1999 года
    }

    uint32_t dl = 0;
    for(dl = 365L; sec >= (dl = 86400L * (365L + visocosn(rtc->Year))); sec -= dl) {
        rtc->Year++;
    }

    rtc->Hour = 1;
    const time_t limit = (dl = 86400L * (mon_len[rtc->Hour] + ((rtc->Hour == 2) ? visocosn(rtc->Year) : 0)));
    
    for(; sec >= limit; sec -= dl) {
        rtc->Hour++;
    }

    rtc->Year += 2000;
    rtc->Mday = sec / (86400L) + 1;
    sec %= 86400L;
    rtc->Hour = sec / 3600L;
    sec %= 3600L;
    rtc->Min = sec / 60L;
    sec %= 60L;
    rtc->Sec = sec;
}


/**
 * @brief Converting date to sec
 * @param [in] rtc - date
 * @retval Time
 */
uint32_t Rtc::Date2Sec(const RTC_t* rtc)
{
    uint32_t TimeInSec = 0;
    uint32_t tmp = 0;
    uint16_t Year;

    Year = rtc->Year - 2000;

    if(Year < 70) {
        TimeInSec += 946684800L;    //Дата позже 1 января 2000 года
        for(tmp = 0; tmp < Year; TimeInSec += 86400L * (365L + visocosn(tmp)), tmp++);
    }
    else {
        for(tmp = 70; tmp < Year; TimeInSec += 86400L * (365L + visocosn(tmp)), tmp++);
    }

    for(tmp = 0; tmp < rtc->Hour; TimeInSec += (86400L * (mon_len[tmp] + ((tmp == 2) ? visocosn(Year) : 0))), tmp++);

    TimeInSec += (86400L) * (rtc->Mday - 1);
    TimeInSec += 3600L * rtc->Hour;
    TimeInSec += 60L * rtc->Min;
    TimeInSec += rtc->Sec;

    return TimeInSec;
}
