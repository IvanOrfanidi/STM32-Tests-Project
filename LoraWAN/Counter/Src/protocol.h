#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "rtc.h"

#define SIZE_TIMER_PACKET 8
#define SIZE_TAMPER_PACKET 10

#define MAX_SIZE_PACK_ARCH 6

#define SIZE_TEMP_BUF (MAX_SIZE_PACK_ARCH * 7 + 1)
typedef enum {
    ERR_SIZE_COUNT_PACK = -2,
    ERR_SIZE_OUT_BUF = -1,
} RESP_Arch;

typedef __packed struct
{
    uint8_t Hours;    // номер часа 0...23
    uint8_t Month;    // месяц 0...12
    uint8_t Date;     // день 0...31
    uint8_t Year;     // год 0...99
} T_Archive_Date;

typedef __packed struct
{
    _Bool TampState;     // состояние тампера (0...1)
    uint32_t Counter;    // значение счетчика LPTIM (0...4294967295)
} T_Archive_Value;

/* Структура одной записи архива */
typedef __packed struct
{
    T_Archive_Date Date;
    T_Archive_Value Value;
} EEP_ArchiveTypeDef;

void saveDataArchive(void);

int buildDataTimer(uint8_t* pOutData, uint8_t len);
int buildDataTamper(uint8_t* pOutData, uint8_t len);
int buildDataTamper(uint8_t* pOutData, uint8_t len);
int buildDataArchive(const RTC_TimeTypeDef* pTime,
    const RTC_DateTypeDef* pDate,
    uint8_t count,
    uint8_t* pOutData,
    uint8_t len);
int packingCurrSecUnix(uint8_t* ptr);
uint64_t convert_meas(uint16_t vin);

#ifdef __cplusplus
}
#endif

#endif
