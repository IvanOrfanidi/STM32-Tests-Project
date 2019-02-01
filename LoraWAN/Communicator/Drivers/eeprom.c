
#include "eeprom.h"
#include "stm32l0xx_hal.h"

uint8_t EE_RD_Byte(uint32_t);
_Bool EE_WR_Byte(uint32_t, uint8_t);

/*
DATA_EEPROM_BASE - ����� ������ EEPROM
DATA_EEPROM_END - ����� ����� EEPROM
*/

/*
������ �� EEPROM(FLASH/RAM) ������ �����.
Input:
  1 - ����� ������ ������ ���� ��������� ����.
Return: �������� ���������� �����.
*/
uint8_t EE_RD_Byte(uint32_t address)
{
    return (*(__IO uint8_t*)(address));
}

/*
������ ����� � EEPROM.
Input:
  1 - ����� ������ ���� ���� �������� ����;
  2 - �������� �����.
Return: ��������� ������.
    0 - ������ � EEPROM �������;
    1 - ������ � EEPROM �� �������.
*/
_Bool EE_WR_Byte(uint32_t address, uint8_t data)
{
    if(EE_RD_Byte(address) != data) {
        HAL_FLASHEx_DATAEEPROM_Unlock();    // ����� ������ EEPROM, ����� ��������� ������
        uint8_t err_write_eeprom = MAX_ERR_EEPROM;
        while(HAL_FLASHEx_DATAEEPROM_Program(TYPEPROGRAMDATA_BYTE, address, data) != HAL_OK) {
            err_write_eeprom--;
            if(!(err_write_eeprom)) {
                HAL_FLASHEx_DATAEEPROM_Lock();    // ������������� ������ EEPROM, ����� ��������� ������
                return 1;
            }
        }
        HAL_FLASHEx_DATAEEPROM_Lock();    // ������������� ������ EEPROM, ����� ��������� ������
    }
    return 0;
}
