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

const uint8_t mon_len[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*******************************************************************************
 * Function Name  : InitRTC
 * Description    : initializes HW RTC,
 *                  sets default time-stamp if RTC has not been initialized
 *before Input          : None Output         : None Return         : not used
 *  Based on code from a STM RTC example in the StdPeriph-Library package
 *******************************************************************************/
void Rtc::InitRTC()
{
    /* Enable PWR and BKP clocks */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);

    /* LSI clock stabilization time */
    for(uint16_t i = 0; i < 5000; i++) {
        ;
    }

    if(BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5) {
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
        while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) {
            Timeout--;
            if(!(Timeout))
                break;
        }

        if(Timeout)    // LSE - OK.
        {
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
        else    // LSE - NO.
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

            // Select the RTC Clock Source
            RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

            /* Enable RTC Clock */
            RCC_RTCCLKCmd(ENABLE);

            /* Wait for RTC registers synchronization */
            RTC_WaitForSynchro();

            /* Wait until last write operation on RTC registers has finished */
            RTC_WaitForLastTask();

            /* Set RTC prescaler: set RTC period to 1sec */
            RTC_SetPrescaler(40000);

            // Error LSE.
        }

        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();

        /* Set initial value */
        RTC_SetCounter((uint32_t)(1262304000));    // here: 1st January 2010 11:55:00

        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();

        BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);

        /* Lock access to BKP Domain */
        PWR_BackupAccessCmd(DISABLE);
    }
    else {
        /* Wait for RTC registers synchronization */
        // RTC_WaitForSynchro();
    }
}

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
        return true;
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

    return false;
}

/**
 * @bref Enable the LSI OSC
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
        return true;
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

    return false;
}

/*******************************************************************************
 * Function Name  : rtc_settime
 * Description    : sets HW-RTC with values from time-struct, takes DST into
 *                  account, HW-RTC always running in non-DST time
 * Input          : None
 * Output         : None
 * Return         : not used
 *******************************************************************************/
void Rtc::SetTime(const RTC_t* const rtc)
{
    uint32_t cnt = Date2Sec(rtc);
    PWR_BackupAccessCmd(ENABLE);

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
    RTC_SetCounter(cnt);

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
    PWR_BackupAccessCmd(DISABLE);
}

void Rtc::GetTime(RTC_t* rtc)
{
    uint32_t t;
    while((t = RTC_GetCounter()) != RTC_GetCounter()) {
        ;
    }
    Sec2Date(rtc, t);
}

void Rtc::Sec2Date(RTC_t* pDest, uint32_t Sec)
{
    uint32_t dl = 0;

    pDest->h12 = RTC_H12_AM;
    pDest->wday = 0;

    if(Sec >= 946684800L) {
        Sec -= 946684800L;    //Дата позже 1 января 2000 года
        pDest->year = 0;
    }
    else {
        pDest->year = 70;    //Дата от 1970 года до 1999 года
    }

    for(dl = 365L; Sec >= (dl = 86400L * (365L + visocosn(pDest->year))); Sec -= dl, pDest->year++)
        ;

    for(pDest->month = 1;
        Sec >= (dl = 86400L * (mon_len[pDest->month] + ((pDest->month == 2) ? visocosn(pDest->year) : 0)));
        Sec -= dl, pDest->month++)
        ;

    pDest->year += 2000;
    pDest->mday = Sec / (86400L) + 1;
    Sec %= 86400L;
    pDest->hour = Sec / 3600L;
    Sec %= 3600L;
    pDest->min = Sec / 60L;
    Sec %= 60L;
    pDest->sec = Sec;
}

uint32_t Rtc::Date2Sec(const RTC_t* pSrc)
{
    uint32_t TimeInSec = 0;
    uint32_t tmp = 0;
    uint16_t year;

    year = pSrc->year - 2000;

    if(year < 70) {
        TimeInSec += 946684800L;    //Дата позже 1 января 2000 года
        for(tmp = 0; tmp < year; TimeInSec += 86400L * (365L + visocosn(tmp)), tmp++)
            ;
    }
    else {
        for(tmp = 70; tmp < year; TimeInSec += 86400L * (365L + visocosn(tmp)), tmp++)
            ;
    }

    for(tmp = 0; tmp < pSrc->month; TimeInSec += (86400L * (mon_len[tmp] + ((tmp == 2) ? visocosn(year) : 0))), tmp++)
        ;

    TimeInSec += (86400L) * (pSrc->mday - 1);
    TimeInSec += 3600L * pSrc->hour;
    TimeInSec += 60L * pSrc->min;
    TimeInSec += pSrc->sec;

    return TimeInSec;
}
