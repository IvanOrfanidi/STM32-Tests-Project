#include "SPI.h"
#ifdef FM3
#include "platform_config_fm3.h"
#include "EXT_FLASH_FM3.h"
#endif
#ifdef FM4
#include "platform_config_fm4.h"
#include "EXT_FLASH_FM4.h"
#endif
#include "stm32l1xx.h"

#ifndef BOOTLOADER
#include "cmsis_os.h"
#include "includes.h"
// uint16_t extern g_usTasksMonitorFlash;
#else
extern void CS_SET(uint8_t num);
extern void CS_FREE(uint8_t num);
#endif

uint8_t SPI_Flash_ReadByte(void)
{
    return SPI1_SendByte(0xFF);
}

void FlashWaitBusy(void)
{
    uint8_t FLASH_Status = 0xFF;
    CS_SET(CS_FLASH);
    SPI1_SendByte(READ_STATUS_REGISTER);
    while(1) {
        IWDG_ReloadCounter(); /* Reload IWDG counter */
        FLASH_Status = SPI_Flash_ReadByte();
        if((FLASH_Status & WIP_Flag) == 0) {
            break;
        }
#ifndef BOOTLOADER
        if(osKernelRunning()) {
            osDelay(10);
        }
#endif
    }
    CS_FREE(CS_FLASH);
}

void FlashReadID(uint8_t* Data)
{
    uint8_t i;

    FlashWaitBusy();

    CS_SET(CS_FLASH);

    SPI1_SendByte(READ_ID);

    for(i = 0; i < 20; i++) {
        Data[i] = SPI_Flash_ReadByte();
    }

    CS_FREE(CS_FLASH);
}

void EXT_FLASH_Read(uint8_t* pBuffer, uint32_t StartAddress, uint16_t Length)
{
    uint16_t i;

    FlashWaitBusy();

    CS_SET(CS_FLASH);

    SPI1_SendByte(FLASH_READ);    // Передаем команду

    // Передаем адрес для чтения.
    SPI1_SendByte((StartAddress & 0xFF0000) >> 16);
    SPI1_SendByte((StartAddress & 0xFF00) >> 8);
    SPI1_SendByte(StartAddress & 0xFF);
    //*************************************//

    // Читаем данные.
    for(i = 0; i < Length; i++) {
        pBuffer[i] = SPI_Flash_ReadByte();
    }
    //*******************************//

    CS_FREE(CS_FLASH);
}

void FlashBulkErase()
{
    FlashWaitBusy();

    CS_SET(CS_FLASH);
    SPI1_SendByte(FLASH_WRITE_ENABLE);
    CS_FREE(CS_FLASH);

    FlashWaitBusy();

    CS_SET(CS_FLASH);

    SPI1_SendByte(BULK_ERASE);

    CS_FREE(CS_FLASH);

    FlashWaitBusy();
}

void FlashSubSectorEarse(uint32_t SubSectorAddress)
{
    FlashWaitBusy();

    CS_SET(CS_FLASH);
    SPI1_SendByte(FLASH_WRITE_ENABLE);
    CS_FREE(CS_FLASH);

    FlashWaitBusy();

    CS_SET(CS_FLASH);

    SPI1_SendByte(SUBSECTOR_ERASE);
    SPI1_SendByte((SubSectorAddress & 0x00FF0000) >> 16);
    SPI1_SendByte((SubSectorAddress & 0xFF00) >> 8);
    SPI1_SendByte(SubSectorAddress & 0xFF);

    CS_FREE(CS_FLASH);
}

void FlashSectorEarse(uint32_t SectorAddress)
{
    FlashWaitBusy();

    CS_SET(CS_FLASH);
    SPI1_SendByte(FLASH_WRITE_ENABLE);
    CS_FREE(CS_FLASH);

    FlashWaitBusy();

    CS_SET(CS_FLASH);

    SPI1_SendByte(SECTOR_ERASE);
    SPI1_SendByte((SectorAddress & 0x00FF0000) >> 16);
    SPI1_SendByte((SectorAddress & 0xFF00) >> 8);
    SPI1_SendByte(SectorAddress & 0xFF);

    CS_FREE(CS_FLASH);
}

void EXT_FLASH_Write(const uint8_t* pBuffer, uint32_t StartAddress, uint16_t Length)
{
    uint16_t i;

    FlashWaitBusy();

    CS_SET(CS_FLASH);
    SPI1_SendByte(FLASH_WRITE_ENABLE);
    CS_FREE(CS_FLASH);

    FlashWaitBusy();

    CS_SET(CS_FLASH);
    SPI1_SendByte(PAGE_PROGRAM);
    // SPI1_SendByte(DUAL_INPUT_FAST_PROGRAM);
    SPI1_SendByte((StartAddress & 0xFF0000) >> 16);
    SPI1_SendByte((StartAddress & 0xFF00) >> 8);
    SPI1_SendByte(StartAddress & 0xFF);

    for(i = 0; i < Length; i++) {
        SPI1_SendByte(pBuffer[i]);
    }
    CS_FREE(CS_FLASH);
}