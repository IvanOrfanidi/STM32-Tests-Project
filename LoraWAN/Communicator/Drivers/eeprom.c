
#include "eeprom.h"
#include "stm32l0xx_hal.h"

uint8_t EE_RD_Byte(uint32_t);
_Bool EE_WR_Byte(uint32_t, uint8_t);

/*
DATA_EEPROM_BASE - јдрес начала EEPROM
DATA_EEPROM_END - јдрес конца EEPROM
*/

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
