#ifndef __RTC_H
#define __RTC_H

#include "includes.h"

/* Clock RTS Source */
//#define RTC_CLOCK_SOURCE_HSE    // Clock HSE 4MHz.
//#define RTC_CLOCK_SOURCE_LSI
#define RTC_CLOCK_SOURCE_LSE
/*****************************/

typedef __packed struct
{
    uint16_t year;    // 1..4095
    uint8_t month;    // 1..12
    uint8_t mday;     // 1..31
    uint8_t wday;     // 0..6, Sunday = 0
    uint8_t hour;     // 0..23
    uint8_t min;      // 0..59
    uint8_t sec;      // 0..59
    uint8_t h12;      // AM..PM
} RTC_t;

int RTC_Configuration(void);

void getSystemDate(RTC_t*);
void setSystemDate(const RTC_t*);

int SetAlarmTime(const RTC_t* date);
void ResetAlarmTime(void);
void GetAlarmTime(RTC_t* date);
uint32_t time(void);

void InitAlarm(void);

void resetSystemDateTime(void);

void SaveDataRTC(void);
void RTC_Configuration_HSE(void);
void RTC_Configuration_LSI(void);

int ReadDataRTC(void);
void SaveDataRTC(void);

#endif
