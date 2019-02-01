#ifndef _GPS_PARSER_H_
#define _GPS_PARSER_H_

#include "includes.h"
#include "stdint.h"

#define USE_GPS_DATA        // ���������� GPS ����������������
#define USE_GLONASS_DATA    // ���������� GLONASS ����������������

typedef enum {
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
    // GPRMC
    uint32_t time;       // ����� ��
    double latitude;     // ������
    double longitude;    // �������
    float course;        // ���� � ����
    float speed;         // �������� � �����
    float hdop;          // hdop

    uint16_t usAltitudeMsl;    //������ � �.
    uint8_t ucUseSat;          // satelites
    uint8_t ucViewSat;
    T_NMEA_MESSAGE_ID eStatus;    // message ID

} GPS_INFO;

void gps_parser(const char* pBuf, const int size);

#endif