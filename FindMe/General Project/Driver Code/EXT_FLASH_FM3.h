

#ifndef _EXT_FLASH_FM3_H_
#define _EXT_FLASH_FM3_H_

#include "platform_config_fm3.h"

// RESET Operations //
#define FLASH_RESET_ENABLE 0x66
#define FLASH_RESET_MEMORY 0x99

// READ Operations //
#define FLASH_READ 0x03
#define FLASH_FAST_READ 0x0B
#define DUAL_OUTPUT_FAST_READ 0x3B
#define QUAD_OUTPUT_FAST_READ 0x6B

// WRITE Operations //
#define FLASH_WRITE_ENABLE 0x06
#define FLASH_WRITE_DISABLE 0x04

// REGISTER Operations //
#define READ_STATUS_REGISTER 0x05    // Read Status Register instruction  //
#define WRITE_STATUS_REGISTER 0x01
#define READ_LOCK_REGISTER 0xE8
#define WRITE_LOCK_REGISTER 0xE5
#define READ_FLAG_STATUS_REGISTER 0x70
#define CLEAR_FLAG_STATUS_REGISTER 0x50
#define READ_NONVOLATILE_CONFIGURATION_REGISTER 0xB5
#define WRITE_NONVOLATILE_CONFIGURATION_REGISTER 0xB1
#define READ_VOLATILE_CONFIGURATION_REGISTER 0x85
#define WRITE_VOLATILE_CONFIGURATION_REGISTER 0x81
#define READ_ENHANCED_VOLATILE_CONFIGURATION_REGISTER 0x65
#define WRITE_ENHANCED_VOLATILE_CONFIGURATION_REGISTER 0x61
#define QUAD_INPUT_FAST_PROGRAM 0x32

// PROGRAM Operations //
#define PAGE_PROGRAM 0x02
#define DUAL_INPUT_FAST_PROGRAM 0xA2

// ERASE Operations //
#define SUBSECTOR_ERASE 0x20
#define SECTOR_ERASE 0xD8    // Sector Erase instruction //
#define BULK_ERASE 0xC7
#define PROGRAM_ERASE_RESUME 0x7A
#define PROGRAM_ERASE_SUSPEND 0x75

// ONE-TIME PROGRAMMABLE (OTP) Operations //
#define READ_OTP_ARRAY 0x4B
#define PROGRAM_OTP_ARRAY 0x42

// IDENTIFICATION Operations //
#define READ_ID 0x9F
#define READ_SERIAL_FLASH_DISCOVERY_PARAMETER 0x5A

#define WIP_Flag \
    0x01 /* Write In Progress (WIP) flag. \
           Indicates if one of the following command cycles is in progress: \
             1)WRITE STATUS REGISTER \
             2)WRITE NONVOLATILE CONFIGURATION REGISTER \
             3)PROGRAM \
             4)ERASE*/

#define WEL_Flag \
    0x02 /* Write enable latch. \
           The device always powers up with this bit \
           cleared to prevent inadvertent WRITE STATUS REGISTER, \
           PROGRAM, or ERASE operations. To enable these operations, the WRITE ENABLE operation must be executed first \
           to set this bit.*/

#define SRWE_Flag \
    0x80 /* Status register write enable/disable. \
           Used with the W#/Vpp signal to enable or \
           disable writing to the status register. \
           A one-time programmable bit used to lock permanently the entire status register. */

uint8_t SPI_Flash_ReadByte(void);
void FlashWaitBusy(void);
void FlashReadID(uint8_t* Data);

void FlashBulkErase();                                  // Full Earse Flash.
void FlashSubSectorEarse(uint32_t SubSectorAddress);    // Max 4096 (4096 bytes, 16 Page).
void FlashSectorEarse(uint32_t SectorAddress);          // Max 256 (65536 bytes, 256 Page).

void EXT_FLASH_Read(uint8_t* pBuffer,
    uint32_t StartAddress,
    uint16_t Length);    // Max Address 16 777 216 and max Length 256.
void EXT_FLASH_Write(const uint8_t* pBuffer,
    uint32_t StartAddress,
    uint16_t Length);    // Max Address 16 777 216 and max Length 256.

/*******************************************************************************

короткий интерфес для доступа к флешке N25Q128A13ESE40F. Передается
номер страницы от 0 до 0xFFFF. Длинна буфера для чтения/записи 256 байт

*******************************************************************************/
#define FLASH_WRITE_PAGE(page, buf) EXT_FLASH_Write(buf, page << 8, 256)
#define FLASH_READ_PAGE(page, buf) EXT_FLASH_Read(buf, page << 8, 256)
#define FLASH_ERASE_SUBSECTOR(subsec) FlashSubSectorEarse(subsec << 12)

#ifdef N25Q128
#define SIZE_EXT_FLASH ((uint32_t)16777216)
#define SIZE_SECTOR_FLASH ((uint32_t)65536)
#endif

#ifdef N25Q64
#define SIZE_EXT_FLASH ((uint32_t)8388608)
#define SIZE_SECTOR_FLASH ((uint32_t)32768)
#endif

#ifdef MX25L1006E
#define SIZE_EXT_FLASH ((uint32_t)131072)
#define SIZE_SECTOR_FLASH ((uint32_t)65536)
#endif

#ifdef MX25L4006E
#define SIZE_EXT_FLASH ((uint32_t)524288)
#define SIZE_SECTOR_FLASH ((uint32_t)65536)
#endif

#define SIZE_SUBSECTOR_FLASH ((uint32_t)4096)
#define SIZE_PAGE_FLASH ((uint32_t)256)

#define FLASH_SUB_SECTOR_PG_NUM SIZE_SUBSECTOR_FLASH / SIZE_PAGE_FLASH    //число страниц в суб секторе

void SaveConfigToFlash(uint8_t* pBuffer, uint16_t Length);
void ReadConfigToFlash(uint8_t* pBuffer, uint16_t Length);
void EraseFirmwareFlash(void);
void EraseArchiveFlash(void);
#endif