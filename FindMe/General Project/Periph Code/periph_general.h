#ifndef _PERIPH_GENERAL_
#define _PERIPH_GENERAL_

#include "includes.h"
#include "eeprom.h"

typedef __packed struct
{
    uint16_t Meas_VIN;
    uint32_t SecADC;
} TPortInputCFG;
GLOBAL TPortInputCFG g_stInput;    //Структура АЦП.

void vPeriphHandler(void* pvParameters);
void PwrHandler(void);
_Bool ChangeStatusDevice(void);
_Bool GpioFilterData(void);
void FillFlashErase(void);
void LedVisual(void);
int8_t GetTemperatur(void);
#endif