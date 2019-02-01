
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PARSER_H
#define __PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MY_PRINTF_
#include "printf-stdarg.h"
#else
#include <stdio.h>
#endif
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE_ARCIVE_PACKET 4
#define SIZE_CONFIG_PACKET 5

int ParseRadio(uint8_t* pInData, uint8_t lenIn, uint8_t* pOutData, uint8_t lenOut);
void ParsingCallback(uint8_t* pData, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif