#ifndef _RTC_H_
#define _RTC_H_
#include "stm32f10x_rtc.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_bkp.h"
#include "stm32f10x.h"

//#include <stdint.h>
//#include <stdbool.h>

typedef struct {
	uint16_t year;	/* 1..4095 */
	uint8_t  month;	/* 1..12 */
	uint8_t  mday;	/* 1.. 31 */
	uint8_t  wday;	/* 0..6, Sunday = 0*/
	uint8_t  hour;	/* 0..23 */
	uint8_t  min;	/* 0..59 */
	uint8_t  sec;	/* 0..59 */
	uint8_t  dst;	/* 0 Winter, !=0 Summer */
        uint8_t  h12;       // AM..PM
} RTC_t;


int rtc_init(void);
void rtc_gettime (RTC_t *rtc);				/* Get time */
void rtc_settime (RTC_t *rtc);				/* Set time */
int rtc_isWork(void);
int RTC_LSE(void);
int RTC_LSI(void);

#endif
