#ifndef __GPS_PARSER_VER2_H
#define __GPS_PARSER_VER2_H

#ifdef __cplusplus
extern "C" {
#endif

#include "includes.h"
#include "stdint.h"

#define USE_GPS_DATA        // ���������� GPS ����������������
#define USE_GLONASS_DATA    // ���������� GLONASS ����������������

typedef __packed struct
{
    uint32_t time;       // ����� ��
    double latitude;     // ������
    double longitude;    // �������
    float course;        // ���� � ����
    float speed;         // �������� � �����
    float hdop;          // hdop
    uint8_t sat;         // ���-�� ���������
    _Bool status;        // A-data valid, V-data not valid
} GPS_INFO;

typedef __packed struct
{
    double latitude;     // ������
    double longitude;    // �������
    float course;        // ����
    float speed;         // ��������

    float hdop;     // hdop
    uint8_t sat;    // satelites
} GPS_INFO_NOT_VALID;

void gps_parser(GPS_INFO* pstGpsInfo, const char* pBuf, const int size);
uint8_t hex2bin(uint8_t c);
#ifdef __cplusplus
}
#endif    //__cplusplus

#endif    //__GPS_PARSER_VER2_H