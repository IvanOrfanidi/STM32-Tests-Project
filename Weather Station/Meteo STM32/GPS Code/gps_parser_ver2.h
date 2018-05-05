#ifndef _GPS_PARSER_VER2_H_
#define _GPS_PARSER_VER2_H_

#include "includes.h"
#include "stdint.h"

#define USE_GPS_DATA   // Используем GPS позиционирование
#define USE_GLONASS_DATA   // Используем GLONASS позиционирование

enum
{
   GPS_RMC = 0,
   GPS_VTG = 1,
   GPS_GGA = 2,
   GPS_GLL = 3,
   GPS_GSA = 4,
   GPS_GSV = 5,
   GPS_TOTAL = 6,
   GPS_START = 7
};

#define MASK_RMC (1 << GPS_RMC)
#define MASK_GGA (1 << GPS_GGA)

typedef enum
{
   NO_MESSAGE_ID = (0 << 0),
   GGA = (1 << 0),
   GLL = (1 << 1),
   GSA = (1 << 2),
   GSV = (1 << 3),
   RMC = (1 << 4),
   VTG = (1 << 5),
   ZDA = (1 << 6),
} T_NMEA_MESSAGE_ID;

typedef __packed struct
{
   uint32_t time;   // время по
   double latitude;   // широта
   double longitude;   // долгота
   float course;   // курс в град
   float speed;   // скорость в милях
   float hdop;   // hdop
   uint8_t sat;   // кол-во спутников
   float altitude;   // высота в метрах
   _Bool status;   // A-data valid, V-data not valid
   uint8_t nmea;   // NMEA message ID
} GPS_INFO;

typedef __packed struct
{
   double latitude;   // широта
   double longitude;   // долгота
   float course;   // курс
   float speed;   // скорость

   float hdop;   // hdop
   uint8_t sat;   // satelites
} GPS_INFO_NOT_VALID;

void gps_parser(GPS_INFO* pstGpsInfo, const char* pBuf, const int size);
uint8_t hex2bin(uint8_t c);

#endif