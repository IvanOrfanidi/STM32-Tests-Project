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
    
    uint8_t Hour;   // 0..23
    uint8_t Min;    // 0..59
    uint8_t Sec;    // 0..59
    
    RTC_t()
    {
        Year = 2010;
        Month = 1;
        Mday = 1;
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

        static void Init(const RTC_t* const rtc = nullptr);    ///< Initializes HW RTC.

        static void SetTime(time_t);

        static void SetTime(const RTC_t* const);    ///< Sets HW-RTC with values from time-struct, takes DST into
                                                    //  account, HW-RTC always running in non-DST time

        static time_t Date2Sec(const RTC_t*);    ///< Converting date to sec

        static void Sec2Date(RTC_t*, time_t);    ///< Convert sec to date

        static void GetTime(RTC_t*);    ///< Geting current time

        static time_t GetTime();     ///< Geting current time

        static bool LseEnable();    ///< LSE Clock 32.768 KHz

        static bool LsiEnable();    ///< LSI Clock ~40 KHz

    private:
    
        static bool InitRtc();
};

#endif

#endif