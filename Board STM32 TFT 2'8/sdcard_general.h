
#ifndef _SDCARD_GENERAL_H_
#define _SDCARD_GENERAL_H_

void OutPutFile(void);
void NVIC_SDIO_Configuration(void);
int SDIO_Fats_Init(void);
void vSdCardTask(void* pvParameters);

#endif