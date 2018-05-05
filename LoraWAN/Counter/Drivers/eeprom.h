#ifndef __EEPROM_H
#define __EEPROM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "protocol.h"
#include "rtc.h"

#define MAX_ERR_EEPROM 3

#define DATA_EEPROM_END (DATA_EEPROM_BANK2_END - 256)

void saveArchiveEeprom(EEP_ArchiveTypeDef* ptrArchive);
_Bool findArchiveEeprom(EEP_ArchiveTypeDef* ptrArchive);

#ifdef __cplusplus
}
#endif

#endif