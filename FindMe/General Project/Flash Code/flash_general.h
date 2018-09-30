
#ifndef __FLASH_GENERAL_H_
#define __FLASH_GENERAL_H_

#include "includes.h"

void vFlashTask(void* pvParameters);
int GetFlashDataToSend(char* pOutDataFrameBuffer, uint32_t SizeOutBuf);
void EraseFirmwareFlash(void);
int GetBufDataToSend(char* pOutDataFrameBuffer);

#endif