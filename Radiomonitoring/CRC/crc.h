#ifndef _CRC_H_
#define _CRC_H_

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>

uint8_t _crc8_update(char* pTxBuffer, int Len);

#endif