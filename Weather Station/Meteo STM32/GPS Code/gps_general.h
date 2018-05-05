
#ifndef _GPS_GENERAL_H_
#define _GPS_GENERAL_H_

#include "includes.h"
#include "gps_parser_ver2.h"

// Выключение не нужных сообщений GPS приемника //
#define GPS_GLL_DISABLE
#define GPS_VTG_DISABLE
#define GPS_GSV_DISABLE
#define GPS_GSA_DISABLE
#define GPS_GGA_DISABLE

// Конфигурирование светодиода GPS приемника //
//  #define LED_CONFIG_ENABLE

// Конфигурирование режима GPS приемника //
//  #define NAVI_AVTO_2D_ENABLE

// Конфигурирование фильтра по остановке в GPS приемнике //
//  #define GPS_FILTER_STOP_ENABLE

#define DELAY_GPS_VALID 10
#define MAX_TIME_NO_VALID_GPS 14400   // 4 hour
#define MAX_TIME_GPS_NO_MUTEX 6

void vGpsHandler(void* pvParameters);
void CpyGpsBuf(const char* pBuf, const int Len);

#endif
