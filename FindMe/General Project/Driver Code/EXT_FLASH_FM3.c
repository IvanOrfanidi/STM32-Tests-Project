#include "SPI.h"
#include "stm32l1xx.h"
#include "EXT_FLASH_FM3.h"

#ifndef BOOTLOADER
#include "cmsis_os.h"
#include "includes.h"
// uint16_t extern g_usTasksMonitorFlash;

void EraseArchiveFlash(void)
{
    for(uint64_t i = 0; i < ADDR_EXT_FLASH_NEW_FIRMWARE; i += SIZE_SECTOR_FLASH) {
        IWDG_ReloadCounter();    // Reload IWDG counter
        FlashSectorEarse(i);
        osDelay(10);
    }
}
#endif

uint8_t SPI_Flash_ReadByte(void)
{
    return SPI2_SendByte(0xFF);
}

void FlashWaitBusy(void)
{
    uint8_t FLASH_Status = 0xFF;
    FLASH_CS_ON;
    SPI2_SendByte(READ_STATUS_REGISTER);
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
    FLASH_CS_OFF;
}

void FlashReadID(uint8_t* Data)
{
    FlashWaitBusy();

    FLASH_CS_ON;
    SPI2_SendByte(READ_ID);
    for(uint8_t i = 0; i < 20; i++) {
        Data[i] = SPI_Flash_ReadByte();
    }
    FLASH_CS_OFF;
}

void EXT_FLASH_Read(uint8_t* pBuffer, uint32_t StartAddress, uint16_t Length)
{
    FlashWaitBusy();

    FLASH_CS_ON;
    SPI2_SendByte(FLASH_READ);    // Передаем команду
    // Передаем адрес для чтения.
    SPI2_SendByte((StartAddress & 0xFF0000) >> 16);
    SPI2_SendByte((StartAddress & 0xFF00) >> 8);
    SPI2_SendByte(StartAddress & 0xFF);
    //*************************************//
    // Читаем данные.
    for(uint16_t i = 0; i < Length; i++) {
        pBuffer[i] = SPI_Flash_ReadByte();
    }
    FLASH_CS_OFF;
}

void FlashBulkErase()
{
    FlashWaitBusy();

    FLASH_CS_ON;
    SPI2_SendByte(FLASH_WRITE_ENABLE);
    FLASH_CS_OFF;

    FlashWaitBusy();

    FLASH_CS_ON;
    SPI2_SendByte(BULK_ERASE);
    FLASH_CS_OFF;
    FlashWaitBusy();

    FLASH_CS_ON;
    SPI2_SendByte(FLASH_WRITE_DISABLE);
    FLASH_CS_OFF;
    FlashWaitBusy();
}

void FlashSubSectorEarse(uint32_t SubSectorAddress)
{
    FlashWaitBusy();

    FLASH_CS_ON;
    SPI2_SendByte(FLASH_WRITE_ENABLE);
    FLASH_CS_OFF;

    FlashWaitBusy();

    FLASH_CS_ON;
    SPI2_SendByte(SUBSECTOR_ERASE);
    SPI2_SendByte((SubSectorAddress & 0x00FF0000) >> 16);
    SPI2_SendByte((SubSectorAddress & 0xFF00) >> 8);
    SPI2_SendByte(SubSectorAddress & 0xFF);
    FLASH_CS_OFF;
    FlashWaitBusy();

    FLASH_CS_ON;
    SPI2_SendByte(FLASH_WRITE_DISABLE);
    FLASH_CS_OFF;
    FlashWaitBusy();
}

void FlashSectorEarse(uint32_t SectorAddress)
{
    FlashWaitBusy();

    FLASH_CS_ON;
    SPI2_SendByte(FLASH_WRITE_ENABLE);
    FLASH_CS_OFF;

    FlashWaitBusy();

    FLASH_CS_ON;
    SPI2_SendByte(SECTOR_ERASE);
    SPI2_SendByte((SectorAddress & 0x00FF0000) >> 16);
    SPI2_SendByte((SectorAddress & 0xFF00) >> 8);
    SPI2_SendByte(SectorAddress & 0xFF);
    FLASH_CS_OFF;
    FlashWaitBusy();

    FLASH_CS_ON;
    SPI2_SendByte(FLASH_WRITE_DISABLE);
    FLASH_CS_OFF;
    FlashWaitBusy();
}

void EXT_FLASH_Write(const uint8_t* pBuffer, uint32_t StartAddress, uint16_t Length)
{
    FlashWaitBusy();

    FLASH_CS_ON;
    SPI2_SendByte(FLASH_WRITE_ENABLE);    //
    FLASH_CS_OFF;

    FlashWaitBusy();

    FLASH_CS_ON;
    SPI2_SendByte(PAGE_PROGRAM);
    // SPI2_SendByte(DUAL_INPUT_FAST_PROGRAM);
    SPI2_SendByte((StartAddress & 0xFF0000) >> 16);
    SPI2_SendByte((StartAddress & 0xFF00) >> 8);
    SPI2_SendByte(StartAddress & 0xFF);

    for(uint16_t i = 0; i < Length; i++) {
        SPI2_SendByte(pBuffer[i]);
    }
    FLASH_CS_OFF;
    FlashWaitBusy();

    FLASH_CS_ON;
    SPI2_SendByte(FLASH_WRITE_DISABLE);
    FLASH_CS_OFF;
    FlashWaitBusy();
}