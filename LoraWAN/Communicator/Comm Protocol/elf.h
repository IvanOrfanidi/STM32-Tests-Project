/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ELF_H
#define __ELF_H

#ifdef __cplusplus

/* Standart functions */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "modbus_elf.h"

/* Mapping Registers Elf Modbus ----------------------------------------------*/
/* Serial Number */
#define REG_ADDR_SER_NUM 0x4203
#define SIZE_REQ_SER_NUM 0x0400

#define NUM_ANS_SER_NUM 13

typedef enum {
    METHOD_SER_NUM = 0,

} TMethod;

/* Структура формата данных запрса */
typedef __packed struct
{
    uint16_t init_reg;    // Initial register
    uint16_t num_reg;     // Number of registers

    uint16_t num_byte;    // ко-во данных в ответе

    TMethod method;    // метод обработки данных ответа
    int size;          // размер выходных данных
} DataElfDef_t;

/* стуктура запроса заводского номера Эльфа */
const static DataElfDef_t stSerialNumber = {
    REG_ADDR_SER_NUM,    // адрес регистра
    SIZE_REQ_SER_NUM,    // размер запрвшиваемых данных

    NUM_ANS_SER_NUM,    // количество данных в ответе

    METHOD_SER_NUM,    // метод которым будут обрабатываться данные ответа от Эльфа

    sizeof(uint32_t)    // размер выходных данных
};

#define REG_ADDR_TEMPER_IN_LINE 0x0E01
#define SIZE_REQ_TEMPER_IN_LINE 0x0400
#define NUM_ANS_TEMPER_IN_LINE 13

const static DataElfDef_t stTemperatureInputPipeline = {
    REG_ADDR_TEMPER_IN_LINE,    // адрес регистра
    SIZE_REQ_TEMPER_IN_LINE,    // размер запрвшиваемых данных

    NUM_ANS_TEMPER_IN_LINE,

    METHOD_SER_NUM,    // метод которым будут обрабатываться данные ответа от Эльфа

    sizeof(uint32_t)    // размер выходных данных
};

#endif /*__cplusplus */
#endif /*__ELF_H*/