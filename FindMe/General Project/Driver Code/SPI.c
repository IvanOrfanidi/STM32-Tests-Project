

#include "SPI.h"
#ifdef FM3
#include "platform_config_fm3.h"
#endif
#ifdef FM4
#include "platform_config_fm4.h"
#endif
#include "stm32l1xx.h"

#ifndef BOOTLOADER
#include "includes.h"

int8_t SPI_Take_Semaphore(void)
{
    if(osKernelRunning()) {
        if(xSemaphoreTake(sBinSPI, portMAX_DELAY) != pdTRUE)
            return FAIL;
    }

    return OK;
}

void SPI_Give_Semaphore(void)
{
    if(osKernelRunning()) {
        xSemaphoreGive(sBinSPI);
    }
}

#endif

void CS_SET(uint8_t num)
{
#ifndef BOOTLOADER
    if(osKernelRunning()) {
        SPI_Take_Semaphore();
    }
#endif

    if(num == CS_FLASH) {
        ACCEL_CS_OFF;
        FLASH_CS_ON;
    }
    else {
        FLASH_CS_OFF;
        ACCEL_CS_ON;
    }
}

void CS_FREE(uint8_t num)
{
    if(num == CS_FLASH) {
        FLASH_CS_OFF;
    }
    else {
        ACCEL_CS_OFF;
    }
#ifndef BOOTLOADER
    if(osKernelRunning()) {
        SPI_Give_Semaphore();
    }
#endif
}

void SPI1_LowLevel_DeInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    SPI_Cmd(SPI1, DISABLE);
    SPI_I2S_DeInit(SPI1);

    // Disable SPI1 Clock //
    RCC_APB2PeriphClockCmd(SPI1_CLK, DISABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;    // GPIO_PuPd_UP;

    /*!< Configure SPI pins: SCK */
    GPIO_InitStructure.GPIO_Pin = SPI1_SCK_PIN;
    GPIO_Init(SPI1_SCK_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure pins: MISO */
    GPIO_InitStructure.GPIO_Pin = SPI1_MISO_PIN;
    GPIO_Init(SPI1_MISO_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure pins: MOSI */
    GPIO_InitStructure.GPIO_Pin = SPI1_MOSI_PIN;
    GPIO_Init(SPI1_MOSI_GPIO_PORT, &GPIO_InitStructure);
}

void SPI2_LowLevel_DeInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    SPI_Cmd(SPI2, DISABLE);
    SPI_I2S_DeInit(SPI2);

    // Disable SPI2 Clock //
    RCC_APB2PeriphClockCmd(SPI2_CLK, DISABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;    // GPIO_PuPd_UP;

    /*!< Configure SPI pins: SCK */
    GPIO_InitStructure.GPIO_Pin = SPI2_SCK_PIN;
    GPIO_Init(SPI2_SCK_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure pins: MISO */
    GPIO_InitStructure.GPIO_Pin = SPI2_MISO_PIN;
    GPIO_Init(SPI2_MISO_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure pins: MOSI */
    GPIO_InitStructure.GPIO_Pin = SPI2_MOSI_PIN;
    GPIO_Init(SPI2_MOSI_GPIO_PORT, &GPIO_InitStructure);
}

#ifdef FM3
void SPI2_LowLevel_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;

    RCC_AHBPeriphClockCmd(FLASH_SPI_CS_GPIO_CLK, ENABLE);

    // Enable SPI1 Clock //
    RCC_APB1PeriphClockCmd(SPI2_CLK, ENABLE);

    // Enable GPIO SPI2 Clock //
    RCC_AHBPeriphClockCmd(SPI2_SCK_GPIO_CLK, ENABLE);
    RCC_AHBPeriphClockCmd(SPI2_MISO_GPIO_CLK, ENABLE);
    RCC_AHBPeriphClockCmd(SPI2_MOSI_GPIO_CLK, ENABLE);
    RCC_AHBPeriphClockCmd(FLASH_SPI_CS_GPIO_CLK, ENABLE);

    /*!< Configure sEE pins: SCK */
    GPIO_InitStructure.GPIO_Pin = SPI2_SCK_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(SPI2_SCK_GPIO_PORT, &GPIO_InitStructure);
    /* Connect PXx to sEE_SPI_SCK */
    GPIO_PinAFConfig(SPI2_SCK_GPIO_PORT, SPI2_SCK_SOURCE, SPI2_SCK_AF);

    /*!< Configure sEE_SPI pins: MISO */
    GPIO_InitStructure.GPIO_Pin = SPI2_MISO_PIN;
    GPIO_Init(SPI2_MISO_GPIO_PORT, &GPIO_InitStructure);
    /* Connect PXx to sEE_SPI_MISO */
    GPIO_PinAFConfig(SPI2_MISO_GPIO_PORT, SPI2_MISO_SOURCE, SPI2_MISO_AF);

    /*!< Configure sEE_SPI pins: MOSI */
    GPIO_InitStructure.GPIO_Pin = SPI2_MOSI_PIN;
    GPIO_Init(SPI2_MOSI_GPIO_PORT, &GPIO_InitStructure);
    /* Connect PXx to sEE_SPI_MOSI */
    GPIO_PinAFConfig(SPI2_MOSI_GPIO_PORT, SPI2_MOSI_SOURCE, SPI2_MOSI_AF);

    /* Configure ACCEL_SPI_CS_PIN pin: sEE_SPI Card CS pin */
    GPIO_InitStructure.GPIO_Pin = FLASH_SPI_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(FLASH_SPI_CS_GPIO_PORT, &GPIO_InitStructure);
    FLASH_CS_OFF;

    //******************************************************//

    // SPI2 Config //
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI2, &SPI_InitStructure);

    // SPI2 enable //
    SPI_Cmd(SPI2, ENABLE);
}

void SPI1_LowLevel_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;

    // Enable SPI1 Clock //
    RCC_APB2PeriphClockCmd(SPI1_CLK, ENABLE);

    // Enable GPIO SPI1 Clock //
    RCC_AHBPeriphClockCmd(SPI1_SCK_GPIO_CLK, ENABLE);
    RCC_AHBPeriphClockCmd(SPI1_MISO_GPIO_CLK, ENABLE);
    RCC_AHBPeriphClockCmd(SPI1_MOSI_GPIO_CLK, ENABLE);
    RCC_AHBPeriphClockCmd(ACCEL_SPI_CS_GPIO_CLK, ENABLE);

    /*!< Configure sEE pins: SCK */
    GPIO_InitStructure.GPIO_Pin = SPI1_SCK_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(SPI1_SCK_GPIO_PORT, &GPIO_InitStructure);
    /* Connect PXx to sEE_SPI_SCK */
    GPIO_PinAFConfig(SPI1_SCK_GPIO_PORT, SPI1_SCK_SOURCE, SPI1_SCK_AF);

    /*!< Configure sEE_SPI pins: MISO */
    GPIO_InitStructure.GPIO_Pin = SPI1_MISO_PIN;
    GPIO_Init(SPI1_MISO_GPIO_PORT, &GPIO_InitStructure);
    /* Connect PXx to sEE_SPI_MISO */
    GPIO_PinAFConfig(SPI1_MISO_GPIO_PORT, SPI1_MISO_SOURCE, SPI1_MISO_AF);

    /*!< Configure sEE_SPI pins: MOSI */
    GPIO_InitStructure.GPIO_Pin = SPI1_MOSI_PIN;
    GPIO_Init(SPI1_MOSI_GPIO_PORT, &GPIO_InitStructure);
    /* Connect PXx to sEE_SPI_MOSI */
    GPIO_PinAFConfig(SPI1_MOSI_GPIO_PORT, SPI1_MOSI_SOURCE, SPI1_MOSI_AF);

    /* Configure ACCEL_SPI_CS_PIN pin: sEE_SPI Card CS pin */
    GPIO_InitStructure.GPIO_Pin = ACCEL_SPI_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(ACCEL_SPI_CS_GPIO_PORT, &GPIO_InitStructure);
    ACCEL_CS_OFF;

    //******************************************************//

    // SPI Config //
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);

    // SPI1 enable //
    SPI_Cmd(SPI1, ENABLE);
}

#endif

#ifdef FM4
void SPI1_LowLevel_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;

    // Enable SPI1 Clock //
    RCC_APB2PeriphClockCmd(SPI1_CLK, ENABLE);

    // Enable GPIO SPI1 Clock //
    RCC_AHBPeriphClockCmd(SPI1_SCK_GPIO_CLK, ENABLE);
    RCC_AHBPeriphClockCmd(SPI1_MISO_GPIO_CLK, ENABLE);
    RCC_AHBPeriphClockCmd(SPI1_MOSI_GPIO_CLK, ENABLE);
    RCC_AHBPeriphClockCmd(FLASH_SPI_CS_GPIO_CLK, ENABLE);
    RCC_AHBPeriphClockCmd(ACCEL_SPI_CS_GPIO_CLK, ENABLE);

    /*!< Configure sEE pins: SCK */
    GPIO_InitStructure.GPIO_Pin = SPI1_SCK_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(SPI1_SCK_GPIO_PORT, &GPIO_InitStructure);
    /* Connect PXx to sEE_SPI_SCK */
    GPIO_PinAFConfig(SPI1_SCK_GPIO_PORT, SPI1_SCK_SOURCE, SPI1_SCK_AF);

    /*!< Configure sEE_SPI pins: MISO */
    GPIO_InitStructure.GPIO_Pin = SPI1_MISO_PIN;
    GPIO_Init(SPI1_MISO_GPIO_PORT, &GPIO_InitStructure);
    /* Connect PXx to sEE_SPI_MISO */
    GPIO_PinAFConfig(SPI1_MISO_GPIO_PORT, SPI1_MISO_SOURCE, SPI1_MISO_AF);

    /*!< Configure sEE_SPI pins: MOSI */
    GPIO_InitStructure.GPIO_Pin = SPI1_MOSI_PIN;
    GPIO_Init(SPI1_MOSI_GPIO_PORT, &GPIO_InitStructure);
    /* Connect PXx to sEE_SPI_MOSI */
    GPIO_PinAFConfig(SPI1_MOSI_GPIO_PORT, SPI1_MOSI_SOURCE, SPI1_MOSI_AF);

    /* Configure FLASH_SPI_CS_PIN pin: sEE_SPI Card CS pin */
    GPIO_InitStructure.GPIO_Pin = FLASH_SPI_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(FLASH_SPI_CS_GPIO_PORT, &GPIO_InitStructure);
    FLASH_CS_OFF;

    /* Configure ACCEL_SPI_CS_PIN pin: sEE_SPI Card CS pin */
    GPIO_InitStructure.GPIO_Pin = ACCEL_SPI_CS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(ACCEL_SPI_CS_GPIO_PORT, &GPIO_InitStructure);
    ACCEL_CS_OFF;

    //******************************************************//

    // SPI Config //
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);

    // SPI1 enable //
    SPI_Cmd(SPI1, ENABLE);
}

/* NO USE FindMe4 */
void SPI2_LowLevel_Init(void)
{
    return;
}

#endif

_Bool flag_spi_fail = 0;
_Bool getFlagSpiFail(void)
{
    return flag_spi_fail;
}

uint8_t SPI1_SendByte(const uint8_t byte)
{
    volatile uint16_t timeout = SPI_TIMEOUT;
    /*!< Loop while DR register in not empty */
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) {
        IWDG_ReloadCounter();
        if(!(timeout--)) {
            flag_spi_fail = 1;
            break;
        }
    }

    /*!< Send byte through the SPI peripheral */
    SPI_SendData(SPI1, byte);

    /*!< Wait to receive a byte */
    timeout = SPI_TIMEOUT;
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) != RESET) {
        IWDG_ReloadCounter();
        if(!(timeout--)) {
            flag_spi_fail = 1;
            break;
        }
    }

    /*!< Return the byte read from the SPI bus */
    return (uint8_t)SPI_ReceiveData(SPI1);
}

uint8_t SPI2_SendByte(const uint8_t byte)
{
    /*!< Loop while DR register in not empty */
    uint16_t timeout = SPI_TIMEOUT;
    while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET) {
        IWDG_ReloadCounter();
        if(!(timeout--)) {
            flag_spi_fail = 1;
            break;
        }
    }

    /*!< Send byte through the SPI peripheral */
    SPI_SendData(SPI2, byte);

    /*!< Wait to receive a byte */
    while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) != RESET) {
        IWDG_ReloadCounter();
        if(!(timeout--)) {
            flag_spi_fail = 1;
            break;
        }
    }

    /*!< Return the byte read from the SPI bus */
    return (uint8_t)SPI_ReceiveData(SPI2);
}
