

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "stm32l1xx.h"
#include "INT_FLASH.h"

#include "stm32l1xx_conf.h"
#include "stm32l1xx_flash.h"

/* Function  flash read word */
// Input Param: address in flash //
// Output Param: data flash in the form of words //
uint32_t flash_read_word(const uint32_t address)
{
   return (*(__IO uint32_t*)address);
}

/* Function flash read buffer */
// Input Param: 1)start address in flash; 2)output buffer data; 3)length buffer //
// Output Param: No //
void flash_read_data(uint8_t* pData_Read, const uint32_t start_address, const uint16_t Len)
{
   uint32_t Data_Read_Word;

   uint16_t address_flash_count = 0;
   for (uint16_t arrey_count = 0; arrey_count < Len;)
   {
      Data_Read_Word = flash_read_word(start_address + (address_flash_count * 4));
      address_flash_count++;

      for (uint8_t i = 0; i < 32; i += 8)
         pData_Read[arrey_count++] = Data_Read_Word >> i;
   }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

/* Function flash write word */
// Input Param: 1)address in flash; 2)data write flash in the form of words //
// Output Param: 1) the sign of successful records (tipe enum FLASH_Status in file "stm32l1xx_flash.h")//
uint8_t flash_write_word(const uint32_t address, const uint32_t Data_Write_Word)
{
   return FLASH_FastProgramWord(address, Data_Write_Word);   //Запись данных.
}

/* Function flash write buffer */
// Input Param: 1)start address in flash; 2)output buffer data for write; 3)length buffer records //
// Output Param: 1) the sign of successful records //
uint8_t flash_write_data(const uint8_t* pData_Write, const uint32_t start_address, const uint16_t Len)
{
   uint32_t uiData_Write_Word;
   // Пишем флеш в обратном порядке байтов, так как чтение также в обратном.
   for (uint16_t address_flash_count = 0; address_flash_count < Len;)
   {
      uiData_Write_Word = 0;
      uint8_t offset = 3;
      for (uint8_t i = 0; i < 4; i++)
      {
         uiData_Write_Word = uiData_Write_Word << 8;
         uiData_Write_Word |= pData_Write[address_flash_count + offset];   // arrey_count
         offset--;
      }
      address_flash_count += 4;
      flash_write_word(start_address + address_flash_count - 4, uiData_Write_Word);
      while (!(FLASH->SR & FLASH_SR_EOP))
      {
         IWDG_ReloadCounter();
      }
      FLASH->SR |= FLASH_SR_EOP;
   }
   return 0;
}

/* Отчиска страницы перед записью в флеш память */
void flash_erase(uint32_t uiPage_Address)
{
   FLASH_ErasePage(uiPage_Address);   //Отчиска страницы перед записью в флешпамять.
   while (!(FLASH->SR & FLASH_SR_EOP))
   {
      IWDG_ReloadCounter();
   }
   FLASH->SR |= FLASH_SR_EOP;   //Сбрасываем этот флаг его установкой в '1'
}