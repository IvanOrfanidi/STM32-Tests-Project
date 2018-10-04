#ifndef _CALENDAR_H
#define _CALENDAR_H

#ifndef RTC_H12_AM
  #define RTC_H12_AM                     ((uint8_t)0x00)
#endif

#ifndef RTC_H12_PM
  #define RTC_H12_PM                     ((uint8_t)0x40)
#endif

void Sec2Date(RTC_t *pDest, uint32_t ulSec);
uint32_t Date2Sec(const RTC_t *pSrc);

#define visocosn(year) ((year % 4) ? 0 : 1)

#define DELTA_ERR_TIME_RTC  60

void TimeSynchronizationRTC(uint32_t uiSec);

int SRV_isCorrect(char *time, char *alarm);

#endif