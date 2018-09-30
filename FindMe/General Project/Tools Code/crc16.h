#ifndef _CRC16_H
#define _CRC16_H

#include <stdint.h>

uint16_t CRC16(const uint8_t* pBuf, uint32_t uiLength);
uint16_t CRC16_FILL(const uint8_t* pBuf, uint32_t uiLength, uint16_t fill);
uint8_t CRC_U8(char* buf, uint16_t len);
uint16_t get_cs16(uint8_t* buf, uint16_t size);
uint8_t CRC8_FILL(uint8_t data, uint8_t fill);
uint8_t egts_crc8(uint8_t* lpBlock, uint8_t len);
uint16_t egts_crc16(uint8_t* pcBlock, uint16_t len);
uint16_t crc16_xmodem(uint8_t* ptr, int count);

#endif
