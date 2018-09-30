#ifndef _AGPS_PROTOCOL_H_
#define _AGPS_PROTOCOL_H_

#define DEF_AGPS_SERVER "online-live1.services.u-blox.com:80"   // NAME SERVER
#define DEF_AGPS_TOKEN "0i8BXXFASUuOh_Mo54032A"   // NAME TOKEN
#define DEF_AGPS_PARAM GNSS_TYPE_GPS | GNSS_TYPE_GLO | DATA_TYPE_EPHEMERIS | DATA_TYPE_ALMANAC   // PARAMETRS

#define TIME_DATA_VALID_AGPS 2 * 60 * 60   // 2 hour valid ephemeris

#define GNSS_TYPE_GPS 0x0001
#define GNSS_TYPE_GLO 0x0002
#define GNSS_TYPE_QZSS 0x0004

#define DATA_TYPE_EPHEMERIS 0x0008
#define DATA_TYPE_ALMANAC 0x0010
#define DATA_TYPE_AUXILIARY 0x0020
#define DATA_TYPE_POSITION 0x0040

#define FILTER_EPHEMERIS_ON 0x0080

RET_INFO GprsSendDataInitAGps(void);
RET_INFO GprsDownloadDataServerAGps(void);
void GprsDownloadDataModulAGps(void);
_Bool UpdateDataAGps(void);   // 1 - Данные AGPS корректны и не требуют обновления, 0 - требуется обновления данных.
void FlashDataAGpsErase(void);

#endif