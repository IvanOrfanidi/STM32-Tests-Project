#include "includes.h"
#include "gps_parser_ver2.h"
#include "gps_tools.h"

int nmea_check_crc(char*);

static void gga_parse(GPS_INFO* pstGpsInfo, const char* pBuf, int size)
{
    float lat = 0;
    float lon = 0;

    float HDOP = 255;
    float altitude = 0;
    char lat_sign = 0x20;
    char lon_sign = 0x20;
    int h = 0;
    int m = 0;
    int s = 0;
    int ms = 0;

    int precision = 0;
    int satellites = 0;

    int crc_error = -1;
    char* pFindGGA = NULL;

    /* Поиск посылки GGA */
#ifdef USE_GPS_DATA
    pFindGGA = strstr(pBuf, "$GPGGA");
    if(pFindGGA) {
        crc_error = nmea_check_crc(pFindGGA);
        pFindGGA += strlen("$GP");
    }
#endif
    /* Если используем ГЛОНАСС данные */
#ifdef USE_GLONASS_DATA
    if(pFindGGA) {
        char* ptr = strstr(pBuf, "$GNGGA");
        if(ptr > NULL) {
            crc_error = nmea_check_crc(ptr);
            pFindGGA = ptr;
            pFindGGA += strlen("$GN");
        }
    }
    else {
        pFindGGA = strstr(pBuf, "$GNGGA");
        if(pFindGGA) {
            crc_error = nmea_check_crc(pFindGGA);
            pFindGGA += strlen("$GN");
        }
    }
#endif

    if((pFindGGA) && (!(crc_error))) {
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
        pstGpsInfo->sat = satellites;
        pstGpsInfo->hdop = HDOP;
    }

    crc_error = -1;
}

static void rmc_parse(GPS_INFO* pstGpsInfo, const char* pBuf, int size)
{
    char* pFindRMC = NULL;

    float lat_seconds = 0;
    float lon_seconds = 0;
    int lat_degrees = 0;
    int lat_minutes = 0;
    int lon_degrees = 0;
    int lon_minutes = 0;

    float lat = 0;
    float lon = 0;

    char lat_sign = 0x20;
    char lon_sign = 0x20;
    int h = 0;
    int m = 0;
    int s = 0;
    int ms = 0;

    float speed = 0;
    float course = 0;

    int year = 0;
    int month = 0;
    int mday = 0;

    char status = 0x20;

    DATE_STRUCT DateGPS;
    int crc_error = -1;

    /* Поиск посылки RMC */
#ifdef USE_GPS_DATA
    pFindRMC = strstr(pBuf, "$GPRMC");
    if(pFindRMC) {
        crc_error = nmea_check_crc(pFindRMC);
        pFindRMC += strlen("$GP");
    }

#endif
    /* Если используем ГЛОНАСС данные */
#ifdef USE_GLONASS_DATA
    if(pFindRMC) {
        char* ptr = strstr(pBuf, "$GNRMC");
        if(ptr) {
            crc_error = nmea_check_crc(ptr);
            pFindRMC = ptr;
            pFindRMC += strlen("$GN");
        }
    }
    else {
        pFindRMC = strstr(pBuf, "$GNRMC");
        if(pFindRMC) {
            crc_error = nmea_check_crc(pFindRMC);
            pFindRMC += strlen("$GN");
        }
    }
#endif

    if((pFindRMC) && (!(crc_error))) {
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
        if(course == 0 && mday == 0 && month == 0 && year == 0) {
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
    }

    /* Приводим координаты к форме выпуска iON Fm */
    if(lat_sign == 'N') {
        lat_degrees += 90;    //Если широта северная, то увеличиваем координаты на 90 градусов.
    }
    if(lon_sign == 'E') {
        lon_degrees += 180;    //Если долгота восточная, то увеличиваем координаты на 180 градусов.
    }

    TIME_STRUCT time;
    _time_from_date(&DateGPS, &time);
    pstGpsInfo->time = time.SEC;

    pstGpsInfo->latitude = (lat_degrees * 100 + lat_minutes) * 10000 + lat_seconds * 100;
    pstGpsInfo->longitude = (lon_degrees * 100 + lon_minutes) * 10000 + lon_seconds * 100;
    pstGpsInfo->course = course;
    pstGpsInfo->speed = speed;

    if(status == 'A') {    //Признак валидности координат
        pstGpsInfo->status = TRUE;
    }
    else {
        pstGpsInfo->status = FALSE;
        pstGpsInfo->latitude = 0;
        pstGpsInfo->longitude = 0;
        pstGpsInfo->course = 0;
        pstGpsInfo->speed = 0;
    }
}

void gps_parser(GPS_INFO* pstGpsInfo, const char* pBuf, const int size)
{
    gga_parse(pstGpsInfo, pBuf, size);

    rmc_parse(pstGpsInfo, pBuf, size);
}

/* преобразование формата ASCII посылки */
uint8_t hex2bin(uint8_t c)
{
    if((c >= 'A') && (c <= 'F')) {
        return c - 'A' + 0xA;
    }
    else if((c >= 'a') && (c <= 'f')) {
        return c - 'a' + 0xA;
    }
    else {
        return c - '0';
    }
}

/* return: -1 - CRC Fail, 0 - CRC OK */
int nmea_check_crc(char* pBuf)
{
    uint8_t crc = 0;
    uint8_t calc_crc = 0;
    pBuf++;

    while(*pBuf != '*') {
        if(*pBuf == '\0') {
            return -1;
        }
        calc_crc ^= *pBuf++;
    }

    crc = hex2bin(*(pBuf + 2)) | hex2bin(*(pBuf + 1)) << 4;
    if(crc != calc_crc) {
        return -1;
    }

    return 0;
}