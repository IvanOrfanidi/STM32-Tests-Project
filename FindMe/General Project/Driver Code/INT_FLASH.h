
#ifndef _INT_FLASH_H_
#define _INT_FLASH_H_

#include <stdint.h>

/* Function flash write word */
// Input Param: 1)address in flash; 2)data write flash in the form of words //
// Output Param: 1) the sign of successful records (tipe enum FLASH_Status in file "stm32l1xx_flash.h")//
uint8_t flash_write_word(const uint32_t address, const uint32_t Data_Write_Word);

/* Function flash write buffer */
// Input Param: 1)start address in flash; 2)output buffer data for write; 3)length buffer records //
// Output Param: 1) the sign of successful records //
uint8_t flash_write_data(const uint8_t* pData_Write, const uint32_t start_address, const uint16_t Len);

/* Function flash read buffer */
// Input Param: 1)start address in flash; 2)output buffer data; 3)length buffer //
// Output Param: No //
void flash_read_data(uint8_t* pData_Read, const uint32_t start_address, const uint16_t Len);

/* Function  flash read word */
// Input Param: address in flash //
// Output Param: data flash in the form of words //
uint32_t flash_read_word(const uint32_t address);
void flash_erase(uint32_t uiPage_Address);
#endif