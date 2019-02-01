
#include "eeprom.h"
#include "protocol.h"
#include "stm32l0xx_hal.h"

uint8_t EE_RD_Byte(uint32_t);
_Bool EE_WR_Byte(uint32_t, uint8_t);

/*
DATA_EEPROM_BASE - јдрес начала EEPROM
DATA_EEPROM_END - јдрес конца EEPROM
*/

/*
—охранение записи в архив EEPROM.
Input: структура данных архива дл€ записи.
Return: no.
*/
void saveArchiveEeprom(EEP_ArchiveTypeDef* ptrArchive)
{
    extern RTC_HandleTypeDef hrtc;
    uint32_t addr_arch_eeprom = HAL_RTCEx_BKUPRead(&hrtc, DEF_REG_COUNT_ARCH_EEP);
    addr_arch_eeprom += sizeof(EEP_ArchiveTypeDef);

    /* Check overflow Eeprom */
    if(addr_arch_eeprom >= DATA_EEPROM_END || addr_arch_eeprom < DATA_EEPROM_BASE) {
        addr_arch_eeprom = DATA_EEPROM_BASE;
    }

    extern _Bool __DEBUG__;
    if(__DEBUG__) {
        printf("Address archive eeprom = 0x%.X\r\n", addr_arch_eeprom);
    }
    HAL_RTCEx_BKUPWrite(&hrtc, DEF_REG_COUNT_ARCH_EEP, addr_arch_eeprom);

    uint8_t* ptr = (uint8_t*)ptrArchive;
    for(int i = 0; i < sizeof(EEP_ArchiveTypeDef); i++) {
        EE_WR_Byte((addr_arch_eeprom + i), ptr[i]);
        if(__DEBUG__) {
            printf("Data write = %d, Data read = %d\r\n", ptr[i], EE_RD_Byte(addr_arch_eeprom + i));
        }
    }
}

/*
¬ыполн€ем поиск записи архива по eeprom.
Input:
  1 - входна€-выходна€ структура в которой передаетс€ дата,
  врем€ и возвращаетс€ значение счетчика и состо€ние тампера в случае успеха.
Return: результат поиска.
        0 - запись найдена в архиве;
        1 - запись в архиве не найдена.
*/
_Bool findArchiveEeprom(EEP_ArchiveTypeDef* ptrArchive)
{
    /* пробегаемс€ по всей eeprom с поиском даты записи */
    uint32_t addr_arch_eeprom = DATA_EEPROM_BASE;
    while(addr_arch_eeprom < DATA_EEPROM_END) {
        /* вычитываем одну запись */
        EEP_ArchiveTypeDef ArchiveSource;
        uint8_t* const ptr = (uint8_t*)&ArchiveSource;
        loop(sizeof(EEP_ArchiveTypeDef))
        {
            if(addr_arch_eeprom > DATA_EEPROM_END)
                break;    //«ащита от переполни€ адреса чтени€ пам€ти.
            ptr[i] = EE_RD_Byte(addr_arch_eeprom++);
        }

        /* сравниваем дату, врем€ */
        if(ptrArchive->Date.Hours == ArchiveSource.Date.Hours && ptrArchive->Date.Date == ArchiveSource.Date.Date &&
            ptrArchive->Date.Month == ArchiveSource.Date.Month && ptrArchive->Date.Year == ArchiveSource.Date.Year) {
            /* запись найдена */
            ptrArchive->Value.Counter = ArchiveSource.Value.Counter;
            ptrArchive->Value.TampState = ArchiveSource.Value.TampState;
            return 0;    // «апись найдена в архиве
        }
    }

    /* «апись в архиве не найдена */
    return 1;
}

/*
„тение из EEPROM(FLASH/RAM) одного байта.
Input:
  1 - адрес пам€ти откуда надо прочитать байт.
Return: значение считанного байта.
*/
uint8_t EE_RD_Byte(uint32_t address)
{
    return (*(__IO uint8_t*)(address));
}

/*
«апись байта в EEPROM.
Input:
  1 - адрес пам€ти куда надо записать байт;
  2 - значение байта.
Return: результат записи.
    0 - запись в EEPROM успешна;
    1 - запись в EEPROM не удалась.
*/
_Bool EE_WR_Byte(uint32_t address, uint8_t data)
{
    if(EE_RD_Byte(address) != data) {
        HAL_FLASHEx_DATAEEPROM_Unlock();    // —н€ть защиту EEPROM, чтобы разрешить запись
        uint8_t err_write_eeprom = MAX_ERR_EEPROM;
        while(HAL_FLASHEx_DATAEEPROM_Program(TYPEPROGRAMDATA_BYTE, address, data) != HAL_OK) {
            err_write_eeprom--;
            if(!(err_write_eeprom)) {
                HAL_FLASHEx_DATAEEPROM_Lock();    // ”станавливаем защиту EEPROM, чтобы запретить запись
                return 1;
            }
        }
        HAL_FLASHEx_DATAEEPROM_Lock();    // ”станавливаем защиту EEPROM, чтобы запретить запись
    }
    return 0;
}
