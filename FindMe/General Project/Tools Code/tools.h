
#ifndef _TOOLS_H_
#define _TOOLS_H_

#include "includes.h"

int bit_packing(char* out, uint32_t inp, uint8_t* bitFree, uint8_t bitUse);
int bit_unpacking(uint8_t* inp, uint32_t* out, uint8_t* bitExist, uint8_t bitTotal);
void DelayResolution100us(uint32_t Dly);

#endif