
#ifndef __SPI_H
#define __SPI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SPI1_CS_DELAY \
   for (uint16_t _ii = 0; _ii < 50; _ii++) \
      ;
#define SPI_TIMEOUT 2000

#define CS_FLASH 0
#define CS_ACCEL 1

void SPI1_LowLevel_Init(void);
void SPI1_LowLevel_DeInit(void);
void SPI2_LowLevel_Init(void);
void SPI2_LowLevel_DeInit(void);
uint8_t SPI1_SendByte(const uint8_t byte);
uint8_t SPI2_SendByte(const uint8_t byte);
#ifndef BOOTLOADER
int8_t SPI_Take_Semaphore(void);
void SPI_Give_Semaphore(void);
void CS_SET(uint8_t num);
void CS_FREE(uint8_t num);
#endif

uint8_t GetShiftReg(uint8_t bit);
void ShiftReg(uint8_t bit, uint8_t val);

_Bool getFlagSpiFail(void);

#ifdef __cplusplus
}
#endif

#endif