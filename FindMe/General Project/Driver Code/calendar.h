#ifndef _CALENDAR_H
#define _CALENDAR_H

void Sec2Date(RTC_t* pDest, uint32_t ulSec);
uint32_t Date2Sec(const RTC_t* pSrc);

#define visocosn(year) ((year % 4) ? 0 : 1)

#define DELTA_ERR_TIME_RTC 60

void TimeSynchronizationRTC(uint32_t SecGPS);

_Bool SerSinchroTime(void);
int SRV_isCorrect(char* time, char* alarm);

#endif