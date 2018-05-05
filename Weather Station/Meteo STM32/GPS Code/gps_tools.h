#ifndef _GPS_H
#define _GPS_H

#include <stdint.h>
#include "includes.h"

typedef struct date_struct
{
   uint16_t YEAR;   // Range from 1970 to 2099.
   uint16_t MONTH;   // Range from 1 to 12.
   uint16_t DAY;   // Range from 1 to 31 (depending on month).
   uint16_t HOUR;   // Range from 0 to 23.
   uint16_t MINUTE;   // Range from 0 to 59.
   uint16_t SECOND;   // Range from 0 to 59.
   uint16_t MILLISEC;   // Range from 0 to 999.
} DATE_STRUCT, *DATE_STRUCT_PTR;

typedef struct time_struct
{
   uint32_t SEC;
   uint32_t MSEC;
} TIME_STRUCT, *TIME_STRUCT_PTR;

// double dist(uint32_t lat_1, uint32_t long_1, uint32_t lat_2, uint32_t long_2);
int _time_from_date(DATE_STRUCT_PTR time_ptr, TIME_STRUCT_PTR ts_ptr);

float ConvertLatitudeGpsFindMe(GPS_INFO* const pGPS_Data);
float ConvertLongitudeGpsFindMe(GPS_INFO* const pGPS_Data);

float ConvertLatitudeGpsAutofon(GPS_INFO* const pGPS_Data);
float ConvertLongitudeGpsAutofon(GPS_INFO* const pGPS_Data);

// расчет расстояния в метров между двумя геоточками
double GetMapDistanceInM(uint32_t Lat1, uint32_t Lon1, uint32_t Lat2, uint32_t Lon2);

#endif
