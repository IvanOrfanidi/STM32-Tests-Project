/**
 ******************************************************************************
 * @file    rtc.hpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    28-March-2018
 * @brief   This file contains all the methods prototypes for the RTC
 *          firmware library.
 ******************************************************************************
 * @attention
 *
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RTC_HPP
#define __RTC_HPP

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_bkp.h"
#include "stm32f10x_rtc.h"

#include <stdint.h>
#include <time.h>
     

#ifdef __cplusplus

     
/*
 * General struct Data Time
 */
struct RTC_t
{
    uint16_t Year;  // 1..4095
    uint8_t Month;  // 1..12
    uint8_t Mday;   // 1.. 31
    uint8_t Wday;   // 0..6(0 - SUNDAY)
    
    uint8_t Hour;   // 0..23
    uint8_t Min;    // 0..59
    uint8_t Sec;    // 0..59
    
    RTC_t()
    {
        Year = 2010;
        Month = 1;
        Mday = 1;
        Wday = 3;
        Hour = 0;
        Min = 0;
        Sec = 0;
    }
    
    enum Defines_t
    { 
        DEF_TIME    = 1262304000u
    };
};


/*
 * @brief Class RTC
 */
class Rtc
{
    public:

        static void Init();     /// Initializes RTC.

        static void SetTime(time_t);    /// Sets RTC current time and date

        static void SetTime(const RTC_t* const);    /// Sets RTC current time and date

        static time_t DateToSec(const RTC_t*);    /// Converting date to sec

        static void SecToDate(RTC_t*, time_t);    /// Convert sec to date

        /// Determines the week number, the day number and the week day number
        static uint8_t GetWeekDayNum(uint32_t, uint8_t, uint8_t);
        
        /// Converts a 2 digit decimal to BCD format
        static uint8_t ByteToBcd2(uint8_t);
        
        /// Converts from 2 digit BCD to Binary
        static uint8_t Bcd2ToByte(uint8_t);

        static void GetTime(RTC_t*);    /// Geting current time

        static time_t GetTime();     /// Geting current time

        enum Default_t
        {
            PREEMPTION_PRIORITY = 0,
            SUB_PRIORITY = 0,
        };

        
        static void InitAlarm(void (*)(), 
                              uint8_t preemption_priority = PREEMPTION_PRIORITY, 
                              uint8_t sub_priority = SUB_PRIORITY);
        
        
        static void SetAlarm(const RTC_t* const);
        
        static void SetAlarm(time_t);
        
        static void ResetAlarm();
        
        static void Handler();      /// Interrupt Handler

    private:
    
        enum : uint16_t
        {
            RTC_BKP_REGISTER    = BKP_DR1,
            RTC_FLAG_ENABLE     = 0xA5A5
        };
        
        
        enum Prescaler_t
        {
            LSE_PRESCALER   = 32767,
            LSI_PRESCALER   = 40000
        };
        
        static void (*Callback)();  ///< Callback handler function
        
        static bool LseEnable();    /// LSE Clock 32.768 KHz

        static bool LsiEnable();    /// LSI Clock ~40 KHz

};

extern "C" {
    /* This Ñ-function handles RTC global interrupt request */
    void RTC_IRQHandler(void);
}

#endif//__cplusplus

#endif