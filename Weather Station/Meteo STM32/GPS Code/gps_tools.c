#include <math.h>
#include <stdio.h>

#include "includes.h"
#include "gps_tools.h"

#define MILLISECS_IN_SECOND (1000)
#define SECS_IN_MINUTE (60UL)
#define MINUTES_IN_HOUR (60UL)
#define HOURS_IN_DAY (24UL)
#define MONTHS_IN_YEAR (12UL)
#define DAYS_IN_YEAR (365UL)

#define CLK_FIRST_YEAR (1970UL)
#define CLK_LAST_YEAR (2239UL)

#define SECS_IN_HOUR (SECS_IN_MINUTE * MINUTES_IN_HOUR)
#define SECS_IN_DAY (SECS_IN_HOUR * HOURS_IN_DAY)
#define SECS_IN_YEAR (SECS_IN_DAY * DAYS_IN_YEAR)

const uint8_t _time_days_in_month_internal[2][13] = { { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
                                                      { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 } };

const uint32_t _time_secs_before_month_internal[2][13] = { { 0,
                                                             31UL * SECS_IN_DAY,
                                                             59UL * SECS_IN_DAY,
                                                             90UL * SECS_IN_DAY,
                                                             120UL * SECS_IN_DAY,
                                                             151UL * SECS_IN_DAY,
                                                             181UL * SECS_IN_DAY,
                                                             212UL * SECS_IN_DAY,
                                                             243UL * SECS_IN_DAY,
                                                             273UL * SECS_IN_DAY,
                                                             304UL * SECS_IN_DAY,
                                                             334UL * SECS_IN_DAY,
                                                             ~0 },
                                                           { 0,
                                                             31UL * SECS_IN_DAY,
                                                             60UL * SECS_IN_DAY,
                                                             91UL * SECS_IN_DAY,
                                                             121UL * SECS_IN_DAY,
                                                             152UL * SECS_IN_DAY,
                                                             182UL * SECS_IN_DAY,
                                                             213UL * SECS_IN_DAY,
                                                             244UL * SECS_IN_DAY,
                                                             274UL * SECS_IN_DAY,
                                                             305UL * SECS_IN_DAY,
                                                             335UL * SECS_IN_DAY,
                                                             ~0 } };

const uint32_t _time_secs_before_year_internal[] = { ((1970UL - 1970UL) * SECS_IN_YEAR) + (0UL * SECS_IN_DAY),
                                                     ((1971UL - 1970UL) * SECS_IN_YEAR) + (0UL * SECS_IN_DAY),
                                                     ((1972UL - 1970UL) * SECS_IN_YEAR) + (0UL * SECS_IN_DAY),
                                                     ((1973UL - 1970UL) * SECS_IN_YEAR) + (1UL * SECS_IN_DAY),
                                                     ((1974UL - 1970UL) * SECS_IN_YEAR) + (1UL * SECS_IN_DAY),
                                                     ((1975UL - 1970UL) * SECS_IN_YEAR) + (1UL * SECS_IN_DAY),
                                                     ((1976UL - 1970UL) * SECS_IN_YEAR) + (1UL * SECS_IN_DAY),
                                                     ((1977UL - 1970UL) * SECS_IN_YEAR) + (2UL * SECS_IN_DAY),
                                                     ((1978UL - 1970UL) * SECS_IN_YEAR) + (2UL * SECS_IN_DAY),
                                                     ((1979UL - 1970UL) * SECS_IN_YEAR) + (2UL * SECS_IN_DAY),
                                                     ((1980UL - 1970UL) * SECS_IN_YEAR) + (2UL * SECS_IN_DAY),
                                                     ((1981UL - 1970UL) * SECS_IN_YEAR) + (3UL * SECS_IN_DAY),
                                                     ((1982UL - 1970UL) * SECS_IN_YEAR) + (3UL * SECS_IN_DAY),
                                                     ((1983UL - 1970UL) * SECS_IN_YEAR) + (3UL * SECS_IN_DAY),
                                                     ((1984UL - 1970UL) * SECS_IN_YEAR) + (3UL * SECS_IN_DAY),
                                                     ((1985UL - 1970UL) * SECS_IN_YEAR) + (4UL * SECS_IN_DAY),
                                                     ((1986UL - 1970UL) * SECS_IN_YEAR) + (4UL * SECS_IN_DAY),
                                                     ((1987UL - 1970UL) * SECS_IN_YEAR) + (4UL * SECS_IN_DAY),
                                                     ((1988UL - 1970UL) * SECS_IN_YEAR) + (4UL * SECS_IN_DAY),
                                                     ((1989UL - 1970UL) * SECS_IN_YEAR) + (5UL * SECS_IN_DAY),
                                                     ((1990UL - 1970UL) * SECS_IN_YEAR) + (5UL * SECS_IN_DAY),
                                                     ((1991UL - 1970UL) * SECS_IN_YEAR) + (5UL * SECS_IN_DAY),
                                                     ((1992UL - 1970UL) * SECS_IN_YEAR) + (5UL * SECS_IN_DAY),
                                                     ((1993UL - 1970UL) * SECS_IN_YEAR) + (6UL * SECS_IN_DAY),
                                                     ((1994UL - 1970UL) * SECS_IN_YEAR) + (6UL * SECS_IN_DAY),
                                                     ((1996UL - 1970UL) * SECS_IN_YEAR) + (6UL * SECS_IN_DAY),
                                                     ((1996UL - 1970UL) * SECS_IN_YEAR) + (6UL * SECS_IN_DAY),
                                                     ((1997UL - 1970UL) * SECS_IN_YEAR) + (7UL * SECS_IN_DAY),
                                                     ((1998UL - 1970UL) * SECS_IN_YEAR) + (7UL * SECS_IN_DAY),
                                                     ((1999UL - 1970UL) * SECS_IN_YEAR) + (7UL * SECS_IN_DAY),
                                                     ((2000UL - 1970UL) * SECS_IN_YEAR) + (7UL * SECS_IN_DAY),
                                                     ((2001UL - 1970UL) * SECS_IN_YEAR) + (8UL * SECS_IN_DAY),
                                                     ((2002UL - 1970UL) * SECS_IN_YEAR) + (8UL * SECS_IN_DAY),
                                                     ((2003UL - 1970UL) * SECS_IN_YEAR) + (8UL * SECS_IN_DAY),
                                                     ((2004UL - 1970UL) * SECS_IN_YEAR) + (8UL * SECS_IN_DAY),
                                                     ((2005UL - 1970UL) * SECS_IN_YEAR) + (9UL * SECS_IN_DAY),
                                                     ((2006UL - 1970UL) * SECS_IN_YEAR) + (9UL * SECS_IN_DAY),
                                                     ((2007UL - 1970UL) * SECS_IN_YEAR) + (9UL * SECS_IN_DAY),
                                                     ((2008UL - 1970UL) * SECS_IN_YEAR) + (9UL * SECS_IN_DAY),
                                                     ((2009UL - 1970UL) * SECS_IN_YEAR) + (10UL * SECS_IN_DAY),
                                                     ((2010UL - 1970UL) * SECS_IN_YEAR) + (10UL * SECS_IN_DAY),
                                                     ((2011UL - 1970UL) * SECS_IN_YEAR) + (10UL * SECS_IN_DAY),
                                                     ((2012UL - 1970UL) * SECS_IN_YEAR) + (10UL * SECS_IN_DAY),
                                                     ((2013UL - 1970UL) * SECS_IN_YEAR) + (11UL * SECS_IN_DAY),
                                                     ((2014UL - 1970UL) * SECS_IN_YEAR) + (11UL * SECS_IN_DAY),
                                                     ((2015UL - 1970UL) * SECS_IN_YEAR) + (11UL * SECS_IN_DAY),
                                                     ((2016UL - 1970UL) * SECS_IN_YEAR) + (11UL * SECS_IN_DAY),
                                                     ((2017UL - 1970UL) * SECS_IN_YEAR) + (12UL * SECS_IN_DAY),
                                                     ((2018UL - 1970UL) * SECS_IN_YEAR) + (12UL * SECS_IN_DAY),
                                                     ((2019UL - 1970UL) * SECS_IN_YEAR) + (12UL * SECS_IN_DAY),
                                                     ((2020UL - 1970UL) * SECS_IN_YEAR) + (12UL * SECS_IN_DAY),
                                                     ((2021UL - 1970UL) * SECS_IN_YEAR) + (13UL * SECS_IN_DAY),
                                                     ((2022UL - 1970UL) * SECS_IN_YEAR) + (13UL * SECS_IN_DAY),
                                                     ((2023UL - 1970UL) * SECS_IN_YEAR) + (13UL * SECS_IN_DAY),
                                                     ((2024UL - 1970UL) * SECS_IN_YEAR) + (13UL * SECS_IN_DAY),
                                                     ((2025UL - 1970UL) * SECS_IN_YEAR) + (14UL * SECS_IN_DAY),
                                                     ((2026UL - 1970UL) * SECS_IN_YEAR) + (14UL * SECS_IN_DAY),
                                                     ((2027UL - 1970UL) * SECS_IN_YEAR) + (14UL * SECS_IN_DAY),
                                                     ((2028UL - 1970UL) * SECS_IN_YEAR) + (14UL * SECS_IN_DAY),
                                                     ((2029UL - 1970UL) * SECS_IN_YEAR) + (15UL * SECS_IN_DAY),
                                                     ((2030UL - 1970UL) * SECS_IN_YEAR) + (15UL * SECS_IN_DAY),
                                                     ((2031UL - 1970UL) * SECS_IN_YEAR) + (15UL * SECS_IN_DAY),
                                                     ((2032UL - 1970UL) * SECS_IN_YEAR) + (15UL * SECS_IN_DAY),
                                                     ((2033UL - 1970UL) * SECS_IN_YEAR) + (16UL * SECS_IN_DAY),
                                                     ((2034UL - 1970UL) * SECS_IN_YEAR) + (16UL * SECS_IN_DAY),
                                                     ((2035UL - 1970UL) * SECS_IN_YEAR) + (16UL * SECS_IN_DAY),
                                                     ((2036UL - 1970UL) * SECS_IN_YEAR) + (16UL * SECS_IN_DAY),
                                                     ((2037UL - 1970UL) * SECS_IN_YEAR) + (17UL * SECS_IN_DAY),
                                                     ((2038UL - 1970UL) * SECS_IN_YEAR) + (17UL * SECS_IN_DAY),
                                                     ((2039UL - 1970UL) * SECS_IN_YEAR) + (17UL * SECS_IN_DAY),
                                                     ((2040UL - 1970UL) * SECS_IN_YEAR) + (17UL * SECS_IN_DAY),
                                                     ((2041UL - 1970UL) * SECS_IN_YEAR) + (18UL * SECS_IN_DAY),
                                                     ((2042UL - 1970UL) * SECS_IN_YEAR) + (18UL * SECS_IN_DAY),
                                                     ((2043UL - 1970UL) * SECS_IN_YEAR) + (18UL * SECS_IN_DAY),
                                                     ((2044UL - 1970UL) * SECS_IN_YEAR) + (18UL * SECS_IN_DAY),
                                                     ((2045UL - 1970UL) * SECS_IN_YEAR) + (19UL * SECS_IN_DAY),
                                                     ((2046UL - 1970UL) * SECS_IN_YEAR) + (19UL * SECS_IN_DAY),
                                                     ((2047UL - 1970UL) * SECS_IN_YEAR) + (19UL * SECS_IN_DAY),
                                                     ((2048UL - 1970UL) * SECS_IN_YEAR) + (19UL * SECS_IN_DAY),
                                                     ((2049UL - 1970UL) * SECS_IN_YEAR) + (20UL * SECS_IN_DAY),
                                                     ((2050UL - 1970UL) * SECS_IN_YEAR) + (20UL * SECS_IN_DAY),
                                                     ((2051UL - 1970UL) * SECS_IN_YEAR) + (20UL * SECS_IN_DAY),
                                                     ((2052UL - 1970UL) * SECS_IN_YEAR) + (20UL * SECS_IN_DAY),
                                                     ((2053UL - 1970UL) * SECS_IN_YEAR) + (21UL * SECS_IN_DAY),
                                                     ((2054UL - 1970UL) * SECS_IN_YEAR) + (21UL * SECS_IN_DAY),
                                                     ((2055UL - 1970UL) * SECS_IN_YEAR) + (21UL * SECS_IN_DAY),
                                                     ((2056UL - 1970UL) * SECS_IN_YEAR) + (21UL * SECS_IN_DAY),
                                                     ((2057UL - 1970UL) * SECS_IN_YEAR) + (22UL * SECS_IN_DAY),
                                                     ((2058UL - 1970UL) * SECS_IN_YEAR) + (22UL * SECS_IN_DAY),
                                                     ((2059UL - 1970UL) * SECS_IN_YEAR) + (22UL * SECS_IN_DAY),
                                                     ((2060UL - 1970UL) * SECS_IN_YEAR) + (22UL * SECS_IN_DAY),
                                                     ((2061UL - 1970UL) * SECS_IN_YEAR) + (23UL * SECS_IN_DAY),
                                                     ((2062UL - 1970UL) * SECS_IN_YEAR) + (23UL * SECS_IN_DAY),
                                                     ((2063UL - 1970UL) * SECS_IN_YEAR) + (23UL * SECS_IN_DAY),
                                                     ((2064UL - 1970UL) * SECS_IN_YEAR) + (23UL * SECS_IN_DAY),
                                                     ((2065UL - 1970UL) * SECS_IN_YEAR) + (24UL * SECS_IN_DAY),
                                                     ((2066UL - 1970UL) * SECS_IN_YEAR) + (24UL * SECS_IN_DAY),
                                                     ((2067UL - 1970UL) * SECS_IN_YEAR) + (24UL * SECS_IN_DAY),
                                                     ((2068UL - 1970UL) * SECS_IN_YEAR) + (24UL * SECS_IN_DAY),
                                                     ((2069UL - 1970UL) * SECS_IN_YEAR) + (25UL * SECS_IN_DAY),
                                                     ((2070UL - 1970UL) * SECS_IN_YEAR) + (25UL * SECS_IN_DAY),
                                                     ((2071UL - 1970UL) * SECS_IN_YEAR) + (25UL * SECS_IN_DAY),
                                                     ((2072UL - 1970UL) * SECS_IN_YEAR) + (25UL * SECS_IN_DAY),
                                                     ((2073UL - 1970UL) * SECS_IN_YEAR) + (26UL * SECS_IN_DAY),
                                                     ((2074UL - 1970UL) * SECS_IN_YEAR) + (26UL * SECS_IN_DAY),
                                                     ((2075UL - 1970UL) * SECS_IN_YEAR) + (26UL * SECS_IN_DAY),
                                                     ((2076UL - 1970UL) * SECS_IN_YEAR) + (26UL * SECS_IN_DAY),
                                                     ((2077UL - 1970UL) * SECS_IN_YEAR) + (27UL * SECS_IN_DAY),
                                                     ((2078UL - 1970UL) * SECS_IN_YEAR) + (27UL * SECS_IN_DAY),
                                                     ((2079UL - 1970UL) * SECS_IN_YEAR) + (27UL * SECS_IN_DAY),
                                                     ((2080UL - 1970UL) * SECS_IN_YEAR) + (27UL * SECS_IN_DAY),
                                                     ((2081UL - 1970UL) * SECS_IN_YEAR) + (28UL * SECS_IN_DAY),
                                                     ((2082UL - 1970UL) * SECS_IN_YEAR) + (28UL * SECS_IN_DAY),
                                                     ((2083UL - 1970UL) * SECS_IN_YEAR) + (28UL * SECS_IN_DAY),
                                                     ((2084UL - 1970UL) * SECS_IN_YEAR) + (28UL * SECS_IN_DAY),
                                                     ((2085UL - 1970UL) * SECS_IN_YEAR) + (29UL * SECS_IN_DAY),
                                                     ((2086UL - 1970UL) * SECS_IN_YEAR) + (29UL * SECS_IN_DAY),
                                                     ((2087UL - 1970UL) * SECS_IN_YEAR) + (29UL * SECS_IN_DAY),
                                                     ((2088UL - 1970UL) * SECS_IN_YEAR) + (29UL * SECS_IN_DAY),
                                                     ((2089UL - 1970UL) * SECS_IN_YEAR) + (30UL * SECS_IN_DAY),
                                                     ((2090UL - 1970UL) * SECS_IN_YEAR) + (30UL * SECS_IN_DAY),
                                                     ((2091UL - 1970UL) * SECS_IN_YEAR) + (30UL * SECS_IN_DAY),
                                                     ((2092UL - 1970UL) * SECS_IN_YEAR) + (30UL * SECS_IN_DAY),
                                                     ((2093UL - 1970UL) * SECS_IN_YEAR) + (31UL * SECS_IN_DAY),
                                                     ((2094UL - 1970UL) * SECS_IN_YEAR) + (31UL * SECS_IN_DAY),
                                                     ((2095UL - 1970UL) * SECS_IN_YEAR) + (31UL * SECS_IN_DAY),
                                                     ((2096UL - 1970UL) * SECS_IN_YEAR) + (31UL * SECS_IN_DAY),
                                                     ((2097UL - 1970UL) * SECS_IN_YEAR) + (32UL * SECS_IN_DAY),
                                                     ((2098UL - 1970UL) * SECS_IN_YEAR) + (32UL * SECS_IN_DAY),
                                                     ((2099UL - 1970UL) * SECS_IN_YEAR) + (32UL * SECS_IN_DAY),
                                                     ~0 };

#define M_PI 3.14159265358979323846

// расчет расстояния в метров между двумя геоточками
double GetMapDistanceInM(uint32_t Lat1, uint32_t Lon1, uint32_t Lat2, uint32_t Lon2)
{
   double Result;

   if (Lat1 == Lat2 && Lon1 == Lon2)
   {
      Result = 0;
   }
   else
   {
      double Diff;
      double ffLat1, ffLat2;
      double ffLon1, ffLon2;
      double kof = M_PI / 180;
      double EARTH_RADIUS = 6384000.0;   // перевели в метры
      char sign[4] = { 0, 0, 0, 0 };
      uint32_t tmp;

      if (Lat1 > 90000000)
         Lat1 -= 90000000;
      else
         sign[0] = 1;
      if (Lat2 > 90000000)
         Lat2 -= 90000000;
      else
         sign[1] = 1;
      if (Lon1 > 180000000)
         Lon1 -= 180000000;
      else
         sign[2] = 1;
      if (Lon2 > 180000000)
         Lon2 -= 180000000;
      else
         sign[3] = 1;

      tmp = Lat1 / 1000000;
      ffLat1 = ((double)tmp + ((double)Lat1 / 10000.0 - (double)tmp * 100.0) / 60.0) * kof;
      tmp = Lat2 / 1000000;
      ffLat2 = ((double)tmp + ((double)Lat2 / 10000.0 - (double)tmp * 100.0) / 60.0) * kof;
      tmp = Lon1 / 1000000;
      ffLon1 = ((double)tmp + ((double)Lon1 / 10000.0 - (double)tmp * 100.0) / 60.0) * kof;
      tmp = Lon2 / 1000000;
      ffLon2 = ((double)tmp + ((double)Lon2 / 10000.0 - (double)tmp * 100.0) / 60.0) * kof;

      if (sign[0])
         ffLat1 = -ffLat1;
      if (sign[1])
         ffLat2 = -ffLat2;
      if (sign[2])
         ffLon1 = -ffLon1;
      if (sign[3])
         ffLon2 = -ffLon2;

      Diff = fabs(ffLon2 - ffLon1);
      if (Diff > M_PI)
      {
         Diff = 2 * M_PI - Diff;
      }

      Result = sin(ffLat2) * sin(ffLat1);
      Result = Result + cos(ffLat2) * cos(ffLat1) * cos(Diff);
      Result = acos(Result);
      Result = EARTH_RADIUS * Result;
   }

   return Result;
}

uint8_t _time_check_if_leap(uint16_t year)
{
   uint8_t leap;

   // If the year is a century year not divisible by 400
   // then it is not a leap year, otherwise if year divisible by
   // four then it is a leap year

   if (year % (uint16_t)100 == (uint16_t)0)
   {
      if (year % (uint16_t)400 == (uint16_t)0)
      {
         leap = 1;
      }
      else
      {
         leap = 0;
      }
   }
   else
   {
      if (year % (uint16_t)4 == (uint16_t)0)
      {
         leap = 1;
      }
      else
      {
         leap = 0;
      }
   }

   return leap;
}

int _time_from_date(DATE_STRUCT_PTR time_ptr, TIME_STRUCT_PTR ts_ptr)
{
   uint32_t time;
   uint8_t leap;

   if ((time_ptr == NULL) || (ts_ptr == NULL))
   {
      return (0);
   }

   if (time_ptr->MILLISEC >= (uint16_t)MILLISECS_IN_SECOND)
   {
      time_ptr->SECOND += (time_ptr->MILLISEC / (uint16_t)MILLISECS_IN_SECOND);
      time_ptr->MILLISEC = (time_ptr->MILLISEC % (uint16_t)MILLISECS_IN_SECOND);
   }

   if (time_ptr->SECOND >= (uint16_t)SECS_IN_MINUTE)
   {
      time_ptr->MINUTE += (time_ptr->SECOND / (uint16_t)SECS_IN_MINUTE);
      time_ptr->SECOND = (time_ptr->SECOND % (uint16_t)SECS_IN_MINUTE);
   }

   if (time_ptr->MINUTE >= (uint16_t)MINUTES_IN_HOUR)
   {
      time_ptr->HOUR += (time_ptr->MINUTE / (uint16_t)60);
      time_ptr->MINUTE = (time_ptr->MINUTE % (uint16_t)60);
   }

   if (time_ptr->HOUR >= (uint16_t)HOURS_IN_DAY)
   {
      time_ptr->DAY += (time_ptr->HOUR / (uint16_t)HOURS_IN_DAY);
      time_ptr->HOUR = (time_ptr->HOUR % (uint16_t)HOURS_IN_DAY);
   }

   if (time_ptr->DAY < 1)
   {
      return (0);
   }

   if (time_ptr->MONTH > (uint16_t)MONTHS_IN_YEAR)
   {
      time_ptr->YEAR += (time_ptr->MONTH / (uint16_t)MONTHS_IN_YEAR);
      time_ptr->MONTH = (time_ptr->MONTH % (uint16_t)MONTHS_IN_YEAR);
   }

   if (time_ptr->YEAR < (uint16_t)CLK_FIRST_YEAR)
   {
      time_ptr->YEAR = (uint16_t)CLK_FIRST_YEAR;
   }

   while (1)
   {
      leap = _time_check_if_leap(time_ptr->YEAR);

      if (time_ptr->DAY > (uint16_t)_time_days_in_month_internal[leap][time_ptr->MONTH])
      {
         time_ptr->DAY -= (uint16_t)_time_days_in_month_internal[leap][time_ptr->MONTH];
         time_ptr->MONTH++;
      }
      else
      {
         break;
      }

      if (time_ptr->MONTH > (uint16_t)MONTHS_IN_YEAR)
      {
         time_ptr->YEAR += (time_ptr->MONTH / (uint16_t)MONTHS_IN_YEAR);
         time_ptr->MONTH = (time_ptr->MONTH % (uint16_t)MONTHS_IN_YEAR);
      }
   }

   if (time_ptr->YEAR > (uint16_t)CLK_LAST_YEAR)
   {
      return (0);
   }

   // Determine the number of seconds since Jan 1, 1970 at 00:00:00
   time = _time_secs_before_year_internal[time_ptr->YEAR - (uint16_t)CLK_FIRST_YEAR];

   // Add the number of seconds_ptr since 0000 hours, Jan 1, to the first day of month.
   time += _time_secs_before_month_internal[leap][time_ptr->MONTH - 1];

   // Add the number of seconds in the days since the beginning of the month
   time += ((uint32_t)time_ptr->DAY - 1) * SECS_IN_DAY;

   // Add the number of seconds in the hours since midnight
   time += (uint32_t)time_ptr->HOUR * SECS_IN_HOUR;

   // Add the number of seconds in the minutes since the hour
   time += (uint32_t)time_ptr->MINUTE * 60;

   // add the number of seconds since the beginning of the minute
   time += (uint32_t)time_ptr->SECOND;

   // assign the times
   ts_ptr->SEC = time;
   ts_ptr->MSEC = (uint32_t)time_ptr->MILLISEC;

   return (1);
}

float ConvertLatitudeGpsFindMe(GPS_INFO* const pGPS_Data)
{
   uint8_t temp_deg;
   uint8_t temp_min;
   float temp_sec;
   float latitude;

   temp_deg = (u8)((pGPS_Data->latitude / 1000000));   //находим градусы.
   temp_min = (u8)((pGPS_Data->latitude - (temp_deg * 1000000)) / 10000);   //находим минуты.
   temp_sec = (pGPS_Data->latitude - (temp_deg * 100 + temp_min) * 10000) / 1000;   //находим сек.

   latitude = (temp_deg - 90) + temp_min / 100.0 + temp_sec / 1000.0;
   latitude *= 100;
   return latitude;
}

float ConvertLongitudeGpsFindMe(GPS_INFO* const pGPS_Data)
{
   uint8_t temp_deg;
   uint8_t temp_min;
   float temp_sec;
   float longitude;

   temp_deg = (u8)(pGPS_Data->longitude / 1000000);   //находим градусы.
   temp_min = (u8)((pGPS_Data->longitude - (temp_deg * 1000000)) / 10000);   //находим минуты.
   temp_sec = (pGPS_Data->longitude - (temp_deg * 100 + temp_min) * 10000) / 1000;   //находим сек.

   longitude = (temp_deg - 180) + temp_min / 100.0 + temp_sec / 1000.0;
   longitude *= 100;
   return longitude;
}

float ConvertLatitudeGpsAutofon(GPS_INFO* const pGPS_Data)
{
   uint8_t temp_deg;
   uint8_t temp_min;
   float temp_sec;
   float latitude;

   temp_deg = (u8)((pGPS_Data->latitude / 1000000));   //находим градусы.
   temp_min = (u8)((pGPS_Data->latitude - (temp_deg * 1000000)) / 10000);   //находим минуты.
   temp_sec = (pGPS_Data->latitude - (temp_deg * 100 + temp_min) * 10000) / 100;   //находим сек.

   latitude = (temp_deg - 90) + temp_min / 60.0 + temp_sec / 6000.0;
   return latitude;
}

float ConvertLongitudeGpsAutofon(GPS_INFO* const pGPS_Data)
{
   uint8_t temp_deg;
   uint8_t temp_min;
   float temp_sec;
   float longitude;

   temp_deg = (u8)(pGPS_Data->longitude / 1000000);   //находим градусы.
   temp_min = (u8)((pGPS_Data->longitude - (temp_deg * 1000000)) / 10000);   //находим минуты.
   temp_sec = (pGPS_Data->longitude - (temp_deg * 100 + temp_min) * 10000) / 100;   //находим сек.

   longitude = (temp_deg - 180) + temp_min / 60.0 + temp_sec / 6000.0;
   return longitude;
}
