#include "includes.h"
#include "new_gps_parser.h"

void gps_parser(const char* pBuf, const int size)
{
   float lat = 0;
   float lon = 0;
   float lat_seconds = 0;
   float lon_seconds = 0;
   float speed = 0, course = 0;
   float HDOP = 255;
   float height = 0;
   char lat_sign = 0x20, lon_sign = 0x20, status = 0x20;
   int h = 0, m = 0, s = 0, ms = 0;
   int lat_degrees = 0, lat_minutes = 0, lon_degrees = 0, lon_minutes = 0, precision = 0, satellites = 0;
   int year = 0, month = 0, mday = 0;
   char pos[128];
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
         pFindRMC += strlen("$GN");
      }
   }
#endif

   if ((pFindGGA > NULL) && (!(crc_error)))
   {
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
             &height);
      lat_degrees = (int)(lat / 100.0);
      lat = lat - lat_degrees * 100;
      lat_minutes = (int)lat;
      lat_seconds = (lat - lat_minutes) * 60;
      lon_degrees = (int)(lon / 100.0);
      lon = lon - lon_degrees * 100;
      lon_minutes = (int)lon;
      lon_seconds = (lon - lon_minutes) * 60;

      sprintf(pos,
              "\r\nGGA: time: %02d:%02d:%02d.%03d; lat: %02d%c%02d'%02.3f\"%c; lon: %02d%c%02d'%02.3f\"%c; height: "
              "%3.1f; %d satellites\r\n",
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
              height,
              satellites);

      USART_Write(UART_DBG, pos, strlen(pos));
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
      lat_degrees = (int)(lat / 100.0);
      lat = lat - lat_degrees * 100;
      lat_minutes = (int)lat;
      lat_seconds = (lat - lat_minutes) * 60;
      lon_degrees = (int)(lon / 100.0);
      lon = lon - lon_degrees * 100;
      lon_minutes = (int)lon;
      lon_seconds = (lon - lon_minutes) * 60;

      sprintf(pos,
              "\r\nRMC: time: %02d:%02d:%02d.%03d; lat: %02d%c%02d'%02.3f\"%c; lon: %02d%c%02d'%02.3f\"%c stat: %c; "
              "sp: %0.1f cours: %0.0f; date: %02d.%02d.%02d;\r\n",
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
              status,
              speed,
              course,
              mday,
              month,
              year);

      USART_Write(UART_DBG, pos, strlen(pos));
   }
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
