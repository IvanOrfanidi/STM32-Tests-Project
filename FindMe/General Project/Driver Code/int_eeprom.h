#ifndef _INT_EEPROM_H
#define _INT_EEPROM_H

#include <stdint.h>
#include "stm32l1xx_flash.h"

#define BASE_EEPROM_ADDRESS 0x08080000

#define END_EEPROM_ADDRESS 0x08080FFF

#define CFG_BASE_ADR 0x00
#define CFG_COPY_BASE_ADR (CFG_BASE_ADR + SIZE_CONFIG_EEPROM)

typedef enum
{
   CMD_EEPROM_SAVE_CONFIG = 0,   // сохранить конфигурацию
   CMD_EEPROM_SAVE_REG_TO_BASE,   // регистраци€ девайса в базу, дл€ сервера 911
   CMD_EEPROM_SAVE_REG_USER,   // регистраци€ пользовател€ в базу, дл€ сервера 911
   CMD_EEPROM_SAVE_START,   // нормальный режим работы, дл€ сервера 911
   CMD_EEPROM_SAVE_ERR_COLD,   // низка€ температура
   CMD_EEPROM_SAVE_GOODBYE,   // смена сервера и протокола, дл€ 911
   CMD_EEPROM_SAVE_ERR_CONSRV,   // неудачный выход на сервер
   CMD_TEST_DEVICE_OK,   // флаг проверки девайса при первом включении
} CMD_EEPROM;
#define SIZE_QUEUE_EEPROM (CMD_EEPROM_SAVE_START + 1)

uint8_t EE_RD_Byte(uint32_t offset);
FLASH_Status EE_WR_Byte(uint32_t offset, uint8_t data);

#ifndef BOOTLOADER
#   include "includes.h"
#   include "eeprom.h"
#   include "ram.h"

void EepromHandler(void);
void SaveConfig(void);
void SaveConfigCMD(void);
_Bool readConfig(void);
void InitConfig(INIT_CONFIG TypeInitConfig);

void ResetInfoFirmware(void);
void SaveTestDataDevice(void);
void ReadTestDataDevice(void);
uint8_t GetStatusDeviceReset(void);
void SetStatusDeviceReset(uint8_t ucResDevStat);
void DSm_ConfigInit(TDSm_DATA* p);
void SetDeviceSleep(uint32_t uiTime);
uint32_t GetDeviceSleep(void);
void SetFlagSleep(void);
_Bool ResetFlagSleep(void);
void ResetFirmware(void);

T_TYPE_CONNECT GetTypeRegBase911(void);   // —ервисы 911
void SetTypeRegBase911(T_TYPE_CONNECT eType);   // —ервисы 911
_Bool GetCheckTestDevice(void);
void SetCheckTestDevice(void);
void deviceDefConfig(void);
#endif

#endif