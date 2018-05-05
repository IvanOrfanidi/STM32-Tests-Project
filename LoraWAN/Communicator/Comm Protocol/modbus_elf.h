/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MODBUS_ELF_H
#define __MODBUS_ELF_H

#ifdef __cplusplus

/* Standart functions */
#   include <stdio.h>
#   include <stdint.h>
#   include <string.h>
#   include <ctype.h>
#   include <stdlib.h>
#   include "elf.h"
#   include "uart.h"

#   define DEF_ADDR_ELF 0x01   // адрес прибора елф modbus, по умолчанию.

#   define FUNC_READ_REG 0x04

#   define ADD_NUM_BYTE 5

#   define TIMEUT_ANSWER 5000U

typedef enum
{
   NOT_ENOUGH_MEMORY = -1,
   OK = 0,
   TIMEOUT_ANSWER = 1,
   CRC_ERROR = 2,
   PARSER_ERROR = 3,
} TYPE_ANS;

/* Структура пакета запроса */
typedef __packed struct
{
   uint8_t addr;   // Address Slave Modbus
   uint8_t funct;   // Function Modbus (0x03, 0x04, 0x10)
   uint16_t init_reg;   // Initial register
   uint16_t num_reg;   // Number of registers
   uint16_t crc;   // CRC
} DEF_RequestElfDef_t;

/* General Class Request */
class RequestElfDef_t
{
 public:
   RequestElfDef_t();

   void setAllRequest(const DEF_RequestElfDef_t*);
   void setAddr(uint8_t);
   void setInitReg(uint16_t);
   void setNumReg(uint8_t);
   void setDataRequest(const DataElfDef_t*);
   int cpyRequest(uint8_t*, int);

   void calculateCrc();

 private:
   DEF_RequestElfDef_t stRequestElf;
};

/* Структура пакета ответа */
typedef __packed struct
{
   uint8_t addr;   // Address Slave Modbus
   uint8_t funct;   // Function Modbus (0x03, 0x04, 0x10)
   uint16_t num_byte;   // количество байт
   TMethod method;   // метод обработки данных ответа
   uint8_t* ptr;   // указатель на буфер данных
   size_t size;   // размер выходных данных
} DEF_AnswerElfDef_t;

/* General Class Answer */
class AnswerElfDef_t
{
 public:
   AnswerElfDef_t();   // Template

   void setDataAnswer(const DataElfDef_t*);
   uint16_t getNumByte() const;

   void setPtr(uint8_t*);
   int parserAnswer(const uint8_t*, uint8_t*);
   int checkCrc(const uint8_t*);
   size_t getSize();

 private:
   void parserMethod(const uint8_t*, uint8_t*);
   DEF_AnswerElfDef_t stAnswerElf;
};

int waitAnswer(uint16_t);

/* General Template */
template<class T>
int pollElf(const DataElfDef_t* ptrDataDef, T* ptr)
{
   /* Create general request Class */
   RequestElfDef_t myRequestElf;

   /* Формируем запрос */
   myRequestElf.setDataRequest(ptrDataDef);
   myRequestElf.calculateCrc();

   /* Получаем запрос */
   uint8_t* ptr_req = new uint8_t[sizeof(DEF_RequestElfDef_t)];
   if (ptr_req == nullptr)
   {
      return NOT_ENOUGH_MEMORY;
   }
   myRequestElf.cpyRequest(ptr_req, sizeof(DEF_RequestElfDef_t));

   /* Отправляем запрос*/
   sendRequest(ptr_req, sizeof(DEF_RequestElfDef_t));
   delete[] ptr_req;

   /* -------------------------------------- */

   /* Create general answer Class */
   AnswerElfDef_t myAnswerElf;
   myAnswerElf.setDataAnswer(ptrDataDef);   // init class

   uint8_t* ptr_ans = new uint8_t[myAnswerElf.getNumByte()];   // create buffer answer
   if (ptr_ans == nullptr)
   {
      return NOT_ENOUGH_MEMORY;
   }

   myAnswerElf.setPtr(ptr_ans);   // set pointer buffer

   /* Ждем определенное количество байт ответа */
   if (waitAnswer(myAnswerElf.getNumByte()))
   {
      delete[] ptr_ans;
      return TIMEOUT_ANSWER;
   }

   /* -------------------------------------- */

   /* Вычитываю данные из буфура UART */
   acceptAnswer(ptr_ans, myAnswerElf.getNumByte());

   /* Проверяем CRC */
   if (myAnswerElf.checkCrc(ptr_ans))
   {
      delete[] ptr_ans;
      return CRC_ERROR;
   }

   /* Create buffer data */
   uint8_t* ptr_data = new uint8_t[myAnswerElf.getSize()];
   if (!(ptr_data))
   {
      delete[] ptr_ans;
      return NOT_ENOUGH_MEMORY;
   }

   /* Парсим ответ */
   if (myAnswerElf.parserAnswer(ptr_ans, ptr_data))
   {
      delete[] ptr_data;
      delete[] ptr_ans;
      return PARSER_ERROR;
   }

   /* помещаем данные в конечный указатель данных */
   size_t index;
   for (index = myAnswerElf.getSize() - 1; index; index--)
   {
      *ptr <<= 8;
      *ptr |= ptr_data[index];
   }
   *ptr <<= 8;
   *ptr |= ptr_data[index];

   delete[] ptr_data;
   delete[] ptr_ans;
   return OK;
}

#endif /*__cplusplus */
#endif /*__MODBUS_ELF_H*/