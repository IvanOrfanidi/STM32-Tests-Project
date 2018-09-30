
#ifndef __SDCARD_GENERAL_H
#define __SDCARD_GENERAL_H

void OutPutFile(void);

void NVIC_SDIO_Configuration(void);

int SDIO_Fats_Init(void);

void vSdCardTask(void* pvParameters);

#endif