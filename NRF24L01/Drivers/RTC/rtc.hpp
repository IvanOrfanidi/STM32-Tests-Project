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

#define visocosn(year) ((year % 4) ? 0 : 1)

#ifndef RTC_H12_AM
#define RTC_H12_AM ((uint8_t)0x00)
#endif

#ifndef RTC_H12_PM
#define RTC_H12_PM ((uint8_t)0x40)
#endif

#ifdef __cplusplus

/*
 * General struct Data Time
 */
#pragma pack(push, 1)
typedef struct
{
    uint16_t year;  /* 1..4095 */
    uint8_t month;  /* 1..12 */
    uint8_t mday;   /* 1.. 31 */
    uint8_t wday;   /* 0..6, Sunday = 0*/
    uint8_t hour;   /* 0..23 */
    uint8_t min;    /* 0..59 */
    uint8_t sec;    /* 0..59 */
    uint8_t dst;    /* 0 Winter, !=0 Summer */
    uint8_t h12;    // AM..PM
} RTC_t;
#pragma pack(pop)

class Rtc
{
  public:
    static void InitRTC();    ///< Initializes HW RTC.

    static void SetTime(const RTC_t* const rtc);    ///< Sets HW-RTC with values from time-struct, takes DST into
                                                    //  account, HW-RTC always running in non-DST time.

    static uint32_t Date2Sec(const RTC_t* pSrc);    ///< Converting date to sec.

    static void Sec2Date(RTC_t*, uint32_t);    ///< Convert sec to date.

    static void GetTime(RTC_t* rtc);    ///< Geting current Time.

    static bool LseEnable();    ///< LSE Clock 32.768 KHz

    static bool LsiEnable();    ///< LSI Clock ~40 KHz

  private:
};

#endif

#endif