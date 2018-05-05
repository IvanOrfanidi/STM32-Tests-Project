#include "includes.h"
#include "gps_parser_ver2.h"
#include "gps_tools.h"

#define DEBUG_GPS_PARSER 0
#ifndef DEBUG_GPS_PARSER
#   define DEBUG_GPS_PARSER 0
#endif

#if (DEBUG_GPS_PARSER == 1)
char pos[200];
#endif

void gps_parser(GPS_INFO* pstGpsInfo, const char* pBuf, const int size)
{
   DATE_STRUCT DateGPS;
   float lat = 0;
   float lon = 0;
   float lat_seconds = 0;
   float lon_seconds = 0;
   float speed = 0, course = 0;
   float HDOP = 255;
   float altitude = 0;
   char lat_sign = 0x20, lon_sign = 0x20, status = 0x20;
   int h = 0, m = 0, s = 0, ms = 0;
   int lat_degrees = 0, lat_minutes = 0, lon_degrees = 0, lon_minutes = 0, precision = 0, satellites = 0;
   int year = 0, month = 0, mday = 0;
   int crc_error = -1;
   char* pFindRMC = NULL;
   char* pFindGGA = NULL;
   int nmea_check_crc(const char* pBuf);

#ifdef USE_GPS_DATA
   pFindGGA = strstr(pBuf, "$GPGGA");
   if (pFindGGA > NULL)
   {
      crc_error = nmea_check_crc(pFindGGA);
      pFindGGA += strlen("$GP");
   }
#endif

#ifdef USE_GLONASS_DATA
   if (pFindGGA > NULL)
   {
      char* ptr = strstr(pBuf, "$GNGGA");
      if (ptr > NULL)
      {
         crc_error = nmea_check_crc(ptr);
         pFindGGA = ptr;
         pFindGGA += strlen("$GN");
      }
   }
   else
   {
      pFindGGA = strstr(pBuf, "$GNGGA");
      if (pFindGGA > NULL)
      {
         crc_error = nmea_check_crc(pFindGGA);
         pFindGGA += strlen("$GN");
      }
   }
#endif

   if ((pFindGGA > NULL) && (!(crc_error)))
   {
#if (DEBUG_GPS_PARSER == 1)
      USART_Write(UART_DBG, pFindGGA, strlen(pFindGGA));
#endif
      sscanf(pFindGGA,
             "GGA,%2d%2d%2d.%3d,%f,%c,%f,%c,%d,%d,%f,%f",
             &h,
             &m,
             &s,
             &ms,
             &lat,
             &lat_sign,
             &lon,
             &lon_sign,
             &precision,
             &satellites,
             &HDOP,
             &altitude);

      lat_degrees = (int)(lat / 100.0);
      lat = lat - lat_degrees * 100;
      lat_minutes = (int)lat;
      lat_seconds = (lat - lat_minutes) * 100;
      lon_degrees = (int)(lon / 100.0);
      lon = lon - lon_degrees * 100;
      lon_minutes = (int)lon;
      lon_seconds = (lon - lon_minutes) * 100;

      pstGpsInfo->sat = satellites;
      pstGpsInfo->hdop = HDOP;
      pstGpsInfo->nmea |= MASK_GGA;
      pstGpsInfo->altitude = altitude;

#if (DEBUG_GPS_PARSER == 1)
      sprintf(pos,
              "\r\nGGA: time: %02d:%02d:%02d.%03d; lat: %02d%c%02d'%02.3f\"%c; lon: %02d%c%02d'%02.3f\"%c; altitude: "
              "%3.1f; hdop: %02.2f; sat: %d\r\n",
              h,
              m,
              s,
              ms,
              lat_degrees,
              0xB0,
              lat_minutes,
              lat_seconds,
              lat_sign,
              lon_degrees,
              0xB0,
              lon_minutes,
              lon_seconds,
              lon_sign,
              altitude,
              HDOP,
              satellites);

      USART_Write(UART_DBG, pos, strlen(pos));
#endif
   }

   crc_error = -1;

#ifdef USE_GPS_DATA
   pFindRMC = strstr(pBuf, "$GPRMC");
   if (pFindRMC > NULL)
   {
      crc_error = nmea_check_crc(pFindRMC);
      pFindRMC += strlen("$GP");
   }

#endif

#ifdef USE_GLONASS_DATA
   if (pFindRMC > NULL)
   {
      char* ptr = strstr(pBuf, "$GNRMC");
      if (ptr > NULL)
      {
         crc_error = nmea_check_crc(ptr);
         pFindRMC = ptr;
         pFindRMC += strlen("$GN");
      }
   }
   else
   {
      pFindRMC = strstr(pBuf, "$GNRMC");
      if (pFindRMC > NULL)
      {
         crc_error = nmea_check_crc(pFindRMC);
         pFindRMC += strlen("$GN");
      }
   }
#endif

   if ((pFindRMC > NULL) && (!(crc_error)))
   {
#if (DEBUG_GPS_PARSER == 1)
      USART_Write(UART_DBG, pFindRMC, strlen(pFindRMC));
#endif

      sscanf(pFindRMC,
             "RMC,%2d%2d%2d.%3d,%c,%f,%c,%f,%c,%f,%f,%2d%2d%2d",
             &h,
             &m,
             &s,
             &ms,
             &status,
             &lat,
             &lat_sign,
             &lon,
             &lon_sign,
             &speed,
             &course,
             &mday,
             &month,
             &year);
      if (course == 0 && mday == 0 && month == 0 && year == 0)
      {
         sscanf(pFindRMC,
                "RMC,%2d%2d%2d.%2d,%c,%f,%c,%f,%c,%f,,%2d%2d%2d",
                &h,
                &m,
                &s,
                &ms,
                &status,
                &lat,
                &lat_sign,
                &lon,
                &lon_sign,
                &speed,
                &mday,
                &month,
                &year);
      }

      lat_degrees = (int)(lat / 100.0);
      lat = lat - lat_degrees * 100;
      lat_minutes = (int)lat;
      lat_seconds = (lat - lat_minutes) * 100;
      lon_degrees = (int)(lon / 100.0);
      lon = lon - lon_degrees * 100;
      lon_minutes = (int)lon;
      lon_seconds = (lon - lon_minutes) * 100;

      DateGPS.MILLISEC = ms;
      DateGPS.SECOND = s;
      DateGPS.MINUTE = m;
      DateGPS.HOUR = h;

      DateGPS.DAY = mday;
      DateGPS.MONTH = month;
      DateGPS.YEAR = year + 2000;

      TIME_STRUCT time;
      _time_from_date(&DateGPS, &time);
      pstGpsInfo->time = time.SEC;

      pstGpsInfo->speed = speed;
      pstGpsInfo->course = course;
      pstGpsInfo->nmea |= MASK_RMC;

      if (status == 'A')
      {
         pstGpsInfo->status = TRUE;
      }
      else
      {
         pstGpsInfo->status = FALSE;
      }

#if (DEBUG_GPS_PARSER == 1)
      sprintf(pos,
              "\r\n\r\nRMC: time: %02d:%02d:%02d.%03d; lat: %02d%c%02d'%02.3f\"%c;",
              h,
              m,
              s,
              ms,
              lat_degrees,
              0xB0,
              lat_minutes,
              lat_seconds,
              lat_sign);
      USART_Write(UART_DBG, pos, strlen(pos));

      sprintf(pos,
              "lon: %02d%c%02d'%02.3f\"%c stat: %c; sp: %0.1f cours: %0.0f; date: %02d.%02d.%02d;\r\n",
              lon_degrees,
              0xB0,
              lon_minutes,
              lon_seconds,
              lon_sign,
              status,
              speed,
              course,
              mday,
              month,
              year);
      USART_Write(UART_DBG, pos, strlen(pos));
#endif
   }
   /* Приводим координаты к форме выпуска iON Fm */
   if (lat_sign == 'N')
   {
      lat_degrees += 90;   //Если широта северная, то увеличиваем координаты на 90 градусов.
   }
   if (lon_sign == 'E')
   {
      lon_degrees += 180;   //Если долгота восточная, то увеличиваем координаты на 180 градусов.
   }
   pstGpsInfo->latitude = (lat_degrees * 100 + lat_minutes) * 10000 + lat_seconds * 100;
   pstGpsInfo->longitude = (lon_degrees * 100 + lon_minutes) * 10000 + lon_seconds * 100;
}

uint8_t hex2bin(uint8_t c)
{
   if ((c >= 'A') && (c <= 'F'))
   {
      return c - 'A' + 0xA;
   }
   else if ((c >= 'a') && (c <= 'f'))
   {
      return c - 'a' + 0xA;
   }
   else
   {
      return c - '0';
   }
}

int nmea_check_crc(const char* pBuf)
{
   uint8_t crc = 0;
   uint8_t calc_crc = 0;
   pBuf++;

   while (*pBuf != '*')
   {
      if (*pBuf == '\0')
      {
         return -1;
      }
      calc_crc ^= *pBuf++;
   }

   crc = hex2bin(*(pBuf + 2)) | hex2bin(*(pBuf + 1)) << 4;
   if (crc != calc_crc)
   {
      return -1;
   }

   return 0;
}