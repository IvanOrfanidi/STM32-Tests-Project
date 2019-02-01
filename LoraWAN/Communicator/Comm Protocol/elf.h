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

/* ��������� ������� ������ ������ */
typedef __packed struct
{
    uint16_t init_reg;    // Initial register
    uint16_t num_reg;     // Number of registers

    uint16_t num_byte;    // ��-�� ������ � ������

    TMethod method;    // ����� ��������� ������ ������
    int size;          // ������ �������� ������
} DataElfDef_t;

/* �������� ������� ���������� ������ ����� */
const static DataElfDef_t stSerialNumber = {
    REG_ADDR_SER_NUM,    // ����� ��������
    SIZE_REQ_SER_NUM,    // ������ ������������� ������

    NUM_ANS_SER_NUM,    // ���������� ������ � ������

    METHOD_SER_NUM,    // ����� ������� ����� �������������� ������ ������ �� �����

    sizeof(uint32_t)    // ������ �������� ������
};

#define REG_ADDR_TEMPER_IN_LINE 0x0E01
#define SIZE_REQ_TEMPER_IN_LINE 0x0400
#define NUM_ANS_TEMPER_IN_LINE 13

const static DataElfDef_t stTemperatureInputPipeline = {
    REG_ADDR_TEMPER_IN_LINE,    // ����� ��������
    SIZE_REQ_TEMPER_IN_LINE,    // ������ ������������� ������

    NUM_ANS_TEMPER_IN_LINE,

    METHOD_SER_NUM,    // ����� ������� ����� �������������� ������ ������ �� �����

    sizeof(uint32_t)    // ������ �������� ������
};

#endif /*__cplusplus */
#endif /*__ELF_H*/