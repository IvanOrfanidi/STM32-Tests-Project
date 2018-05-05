

/* Includes ------------------------------------------------------------------*/
#include "modbus_elf.h"
#include <stdint.h>
#include <stdlib.h>
#include <cmath>
#include "crc.hpp"

#include "elf.h"

#include <iostream>
using namespace std;

/* Template Request */
/* Шаблон запроса данных */
RequestElfDef_t::RequestElfDef_t()
{
   stRequestElf.addr = DEF_ADDR_ELF;
   stRequestElf.funct = FUNC_READ_REG;
   stRequestElf.crc = 0xFFFF;
}

void RequestElfDef_t::setAllRequest(const DEF_RequestElfDef_t* ptr)
{
   memcpy(&stRequestElf, ptr, sizeof(RequestElfDef_t));
}

void RequestElfDef_t::setAddr(uint8_t addr)
{
   stRequestElf.addr = addr;
}

void RequestElfDef_t::setInitReg(uint16_t init_reg)
{
   stRequestElf.init_reg = init_reg;
}

void RequestElfDef_t::setNumReg(uint8_t num_reg)
{
   stRequestElf.num_reg = num_reg;
}

void RequestElfDef_t::setDataRequest(const DataElfDef_t* ptr)
{
   stRequestElf.init_reg = ptr->init_reg;
   stRequestElf.num_reg = ptr->num_reg;
}

/* Вычисление CRC */
void RequestElfDef_t::calculateCrc()
{
   uint8_t* ptr = new uint8_t[sizeof(stRequestElf)];
   if (ptr == nullptr)
   {
      cout << "Error Memory!\r\n";
   }
   memcpy(ptr, &stRequestElf, sizeof(stRequestElf));
   stRequestElf.crc = CRC::Modbus(ptr, sizeof(stRequestElf));
   delete ptr;
}

/* Копирование запроса */
int RequestElfDef_t::cpyRequest(uint8_t* ptr, int size)
{
   if (size != sizeof(DEF_RequestElfDef_t))
   {
      return -1;
   }
   memcpy(ptr, &stRequestElf, sizeof(DEF_RequestElfDef_t));
   return 0;
}

/******************************************************************************/

/* Template Answer */
/* Шаблон ответа данных */
AnswerElfDef_t::AnswerElfDef_t()
{
   stAnswerElf.addr = DEF_ADDR_ELF;
   stAnswerElf.funct = FUNC_READ_REG;
}

uint16_t AnswerElfDef_t::getNumByte() const
{
   return stAnswerElf.num_byte;
}

void AnswerElfDef_t::setDataAnswer(const DataElfDef_t* ptr)
{
   stAnswerElf.num_byte = ptr->num_byte;
   stAnswerElf.method = ptr->method;
   stAnswerElf.size = ptr->size;
}

void AnswerElfDef_t::setPtr(uint8_t* ptr)
{
   stAnswerElf.ptr = ptr;
}

int AnswerElfDef_t::checkCrc(const uint8_t* ptr)
{
   /* Проверка CRC */
   uint16_t crc = CRC::Modbus(ptr, stAnswerElf.num_byte);
   uint8_t crc_l = uint8_t(crc & 0x00FF);
   uint8_t crc_h = uint8_t(crc >> 8);
   if (ptr[stAnswerElf.num_byte - 1] != crc_h || ptr[stAnswerElf.num_byte - 2] != crc_l)
   {
      return -1;   // CRC FAIL
   }
   return 0;   // CRC OK
}

static uint64_t data2buf(const uint8_t* ptr, uint16_t len)
{
   uint64_t value;
   int n = 0;
   for (uint16_t i = len; i; i--, n++)
   {
      if (n)
      {
         value += (ptr[i] * pow((double)10, n));
      }
      else
      {
         value = ptr[i];
      }
   }
   return value;
}

void AnswerElfDef_t::parserMethod(const uint8_t* ptr, uint8_t* ptr_data)
{
   /* переставляем байты регистров местами */
   uint8_t temp_buf[32] = { 0 };
   for (uint8_t i = 0; i < stAnswerElf.num_byte - 5; i += 2)
   {
      temp_buf[i] = ptr[i + 1];
      temp_buf[i + 1] = ptr[i];
   }

   uint64_t temp_data = data2buf(temp_buf, (stAnswerElf.num_byte - 6));
   loop(getSize())
   {
      uint8_t byte = temp_data;
      ptr_data[i] = byte;
      temp_data >>= 8;
   }
}

int AnswerElfDef_t::parserAnswer(const uint8_t* ptr, uint8_t* ptr_data)
{
   /* Проверка адресата */
   if (stAnswerElf.addr != ptr[0] || stAnswerElf.funct != ptr[1] || stAnswerElf.num_byte - 5 != ptr[2])
   {
      return -1;
   }

   switch (stAnswerElf.method)
   {
   case METHOD_SER_NUM:
      parserMethod(ptr + 3, ptr_data);
      break;
   }
   return 0;
}

size_t AnswerElfDef_t::getSize()
{
   return stAnswerElf.size;
}

/******************************************************************************/
int waitAnswer(uint16_t num_byte)
{
   int timeout = TIMEUT_ANSWER;
   while (lenCommRxBuf() != num_byte)
   {
      timeout--;
      if (!(timeout))
      {
         return 1;
      }
      HAL_Delay(1);
   }
   return 0;
}
