

#include "includes.h"
#include "rtc.h"
#include "calendar.h"

const uint8_t mon_len[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

void Sec2Date(RTC_t *pDest, uint32_t ulSec)
{
  uint32_t  dl = 0;
  
  pDest->h12 = RTC_H12_AM;
  pDest->wday = 0;
  
  if ( ulSec >= 946684800L ) {
    ulSec -= 946684800L; //Дата позже 1 января 2000 года
    pDest->year = 0;
   }
   else {
      pDest->year = 70;//Дата от 1970 года до 1999 года
   }

  for ( dl = 365L;
    ulSec >= (dl = 86400L * (365L + visocosn(pDest->year))) ;
    ulSec -= dl, pDest->year++ );

  for ( pDest->month = 1;
    ulSec >= ( dl = 86400L *
    (mon_len[pDest->month] + ((pDest->month == 2) ?
       visocosn(pDest->year) : 0)));  ulSec -= dl, pDest->month++);

  pDest->year+=2000;
  pDest->mday = ulSec / (86400L) + 1;
  ulSec %= 86400L;
  pDest->hour = ulSec / 3600L;
  ulSec %= 3600L;
  pDest->min = ulSec / 60L;
  ulSec %= 60L;
  pDest->sec = ulSec;
}

uint32_t Date2Sec(const RTC_t *pSrc)
{
  uint32_t ulTimeInSec = 0;
  uint32_t  tmp = 0;
  uint16_t year;
  
  year = pSrc->year - 2000;

  if ( year < 70) {
    ulTimeInSec += 946684800L; //Дата позже 1 января 2000 года
    for (tmp = 0;
      tmp < year;
      ulTimeInSec += 86400L * (365L + visocosn(tmp)), tmp++ );
  }
  else
  {
    for (tmp = 70;
      tmp < year;
      ulTimeInSec += 86400L * (365L + visocosn(tmp)), tmp++ );
  }
  
  for ( tmp = 0;
          tmp < pSrc->month;
            ulTimeInSec += ( 86400L *
            (mon_len[tmp] + ((tmp == 2) ?  visocosn(year) : 0))),
            tmp++);

  ulTimeInSec  += (86400L)*(pSrc->mday-1);
  ulTimeInSec += 3600L*pSrc->hour;
  ulTimeInSec += 60L*pSrc->min;
  ulTimeInSec += pSrc->sec;
  
  return ulTimeInSec;
}


// Функция корекции времени RTC по времени GPS. !Время в RTC идет по UTC.
void TimeSynchronizationRTC(uint32_t uiSec)
{  
   if(uiSec < 1427464200) {
     return;
   }
  
  RTC_t DateRTC;
  uint32_t SecRTC;
  
  rtc_gettime(&DateRTC);
  SecRTC = Date2Sec(&DateRTC);
  
  if(SecRTC < uiSec - DELTA_ERR_TIME_RTC || SecRTC > uiSec + DELTA_ERR_TIME_RTC)
  {
      Sec2Date(&DateRTC,uiSec);
      rtc_settime(&DateRTC);      
  }
}

uint8_t str2_to_int8(char *s)
{
    return ((s[0]-'0')*10 + (s[1]-'0'));
}


// проверка даты на корректность
int rtcDATE_isCorrect(RTC_t *date)
{
    register signed char res=0;
    
    if(date->year < 100) {
        switch(date->month)
        {
            case 1: case 3: case 5: case 7:
            case 8: case 10: case 12:
                if(date->mday  > 31) res=-1;
                break;
                
            case 4: case 6: case 9: case 11:
                if(date->mday  > 30) res=-1;
                break;
                
            case 2:
                if((date->year%4) == 0)
                {   
                    if(date->mday  > 29) res=-1;
                    break;
                }
    
                if(date->mday > 28) res=-1;
                break;
                
            default:
                res=-1;
        }
    } else {
        res=-1;
    }
    
    return res;
}

// проверка даты на корректность
int strDate_IsCorrect(char *buf, RTC_t *date)
{
    char n;
    
    if(buf != 0) {
        for(n=0; n<11; n++) {
            if(buf[n] < '0' || buf[n] > '9') { 
                return -1;
            }
        }
        
        date->year  = str2_to_int8(buf);
        date->month = str2_to_int8(buf + 2);
        date->mday  = str2_to_int8(buf + 4);
        date->hour  = str2_to_int8(buf + 6);
        date->min   = str2_to_int8(buf + 8);
        date->wday  = buf[10]-'0';

        if(date->hour < 24 && date->min < 60 && date->wday < 7) {
            if(rtcDATE_isCorrect(date) == 0) {
                return 0;
            }
        }
    }
    
    return -1;
}

int SRV_isCorrect(char *time, char *alarm)
{
    RTC_t date;
    
    if(strDate_IsCorrect(time, &date) == 0)
    {   if(strDate_IsCorrect(alarm, &date) == 0)
        {   return 0;
        }
    }
    return -1;
}
