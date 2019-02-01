#ifndef __GPS_PARSER_VER2_H
#define __GPS_PARSER_VER2_H

#ifdef __cplusplus
extern "C" {
#endif

#include "includes.h"
#include "stdint.h"

#define USE_GPS_DATA        // Используем GPS позиционирование
#define USE_GLONASS_DATA    // Используем GLONASS позиционирование

typedef __packed struct
{
    uint32_t time;       // время по
    double latitude;     // широта
    double longitude;    // долгота
    float course;        // курс в град
    float speed;         // скорость в милях
    float hdop;          // hdop
    uint8_t sat;         // кол-во спутников
    _Bool status;        // A-data valid, V-data not valid
} GPS_INFO;

typedef __packed struct
{
    double latitude;     // широта
    double longitude;    // долгота
    float course;        // курс
    float speed;         // скорость

    float hdop;     // hdop
    uint8_t sat;    // satelites
} GPS_INFO_NOT_VALID;

void gps_parser(GPS_INFO* pstGpsInfo, const char* pBuf, const int size);
uint8_t hex2bin(uint8_t c);
#ifdef __cplusplus
}
#endif    //__cplusplus

#endif    //__GPS_PARSER_VER2_H