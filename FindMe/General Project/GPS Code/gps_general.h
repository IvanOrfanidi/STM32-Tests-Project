
#ifndef __GPS_GENERAL_H
#define __GPS_GENERAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "includes.h"

// Выключение не нужных сообщений GPS приемника //
#define GPS_GGA_ENABLE 1
#define GPS_RMC_ENABLE 1
#define GPS_GLL_ENABLE 0
#define GPS_VTG_ENABLE 0
#define GPS_GSV_ENABLE 0
#define GPS_GSA_ENABLE 0
#define GPS_ZDA_ENABLE 0
#define GPS_ACCURACY_ENABLE 0

#define AUTO_SET_BAUDRATE_GPS 1
#if !(AUTO_SET_BAUDRATE_GPS)
#define DEF_GPS_BAUDRATE 115200
#endif

#define GPS_VALID \
    (g_stGpsData.latitude && g_stGpsData.longitude && g_stGpsData.time && g_stGpsData.sat && g_stGpsData.status)

// Конфигурирование светодиода GPS приемника //
//  #define LED_CONFIG_ENABLE

// Конфигурирование режима GPS приемника //
//  #define NAVI_AVTO_2D_ENABLE

// Конфигурирование фильтра по остановке в GPS приемнике //
#define GPS_FILTER_STOP_ENABLE

#define TIME_NO_VALID_GPS 3600    // 4 часа(пока)

void vGpsHandler(void* pvParameters);

int TakeGpsData(char* pInpDataFrameBuffer);
_Bool GetGpsStatus(void);
_Bool GpsFilterData(void);
_Bool GpsRealTimeFilterData(void);
void ReloadGpsRealTime(void);
uint32_t GetDataNavigationalGpsPacket(char* pOutDataFrameBuffer, uint8_t TypePacket, uint32_t OffsetData);
void GpsPowerMonitor(void);
_Bool GpsFilterDataConfig(uint8_t MinSpeed,
    uint16_t GpsDistance,
    _Bool GpsAccelTrue,
    uint16_t GpsTime,
    uint8_t GpsCourse);
int FindPositionGps(void);
_Bool GetPositionGps(GPS_INFO* ptrGpsData);
void CpyGpsBuf(const char* pBuf, int Len);
void clearTimeFilterGps();
float getHdopGps(void);
void saveOldPositionGps(void);

#ifdef __cplusplus
}
#endif

#endif
