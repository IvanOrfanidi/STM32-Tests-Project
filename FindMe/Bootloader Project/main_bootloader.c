
#include <intrinsics.h>
#include "stm32l1xx.h"
#include "arm_comm.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <intrinsics.h>
#include <assert.h>

#ifdef FM4
#   include "EXT_FLASH_FM4.h"
#endif
#ifdef FM3
#   include "EXT_FLASH_FM3.h"
#endif
#include "INT_FLASH.h"
#include "SPI.h"
#include "crc16.h"
#include "eeprom.h"

#define BOOT_VER "01"   //Версия bootloader
#pragma location = "ConstBootVer"
__root const char US_BOOT_VER[2] = BOOT_VER;

typedef void (*pFunction)(void);
pFunction Jump_To_Application;

#define RET_OK 0
#define ERR_EXT_FLASH 1
#define ERR_INT_FLASH 2

uint8_t DataBoot[SIZE_RECORD_EXT_FLASH];

#define LED_TIME 4
#define TIMEOUT 1000

uint8_t LedStatus = 0;
#define LED_ON_OFF \
   if (LedStatus < LED_TIME) \
      LED_TOGGLE; \
   if (LedStatus > LED_TIME * 2) \
      LedStatus = 0; \
   LedStatus++;

uint32_t uiNameNewFirmware = 0;   //Имя новой прошивки во внешней flash.

static void delay(int iTimeout);

FIRMWARE_TYPE BootloaderFirmwareTrue(void)
{
   char* p1 = (char*)&uiNameNewFirmware;
   char* p2 = (char*)&DataBoot[240];
   uint32_t uiAdd = (ADDR_EXT_FLASH_NEW_FIRMWARE + TOTAL_SIZE_FIRMWARE) - SIZE_RECORD_EXT_FLASH;
   EXT_FLASH_Read(DataBoot, uiAdd, SIZE_RECORD_EXT_FLASH);
   for (uint8_t i = 0; i < sizeof(uiNameNewFirmware); i++)
   {
      p1[i] = p2[i];
   }

   if (uiNameNewFirmware == 0xFFFFFFFF)
   {
      return NO_BASE_FIRMWARE;
   }

   if (uiNameNewFirmware != flash_read_word(__CONST_FIRM_VER))
   {
      return UPDATE_FIRMWARE;
   }

   return NO_NEW_FIRMWARE;
}

int strarr(const uint8_t* pArrey1, const uint8_t* pArrey2, uint16_t SizeArr)
{
   for (uint16_t i = 0; i < SizeArr; i++)
   {
      if (pArrey1[i] != pArrey2[i])
      {
         return 1;
      }
   }
   return 0;
}

// Обновление прошивки
int UpdateNewFirmware(void)
{
   uint32_t StartAddressExtFlash = ADDR_EXT_FLASH_NEW_FIRMWARE;
   uint32_t StartAddressIntFlash = START_ADDRESS_CODE_INT_FLASH;
   uint16_t CountPageData = COUNT_FLASH_PAGE;

   FLASH_Unlock();
   while (CountPageData)
   {
      IWDG_ReloadCounter();
      LED_ON_OFF;

      EXT_FLASH_Read(DataBoot, StartAddressExtFlash, SIZE_RECORD_EXT_FLASH);

      flash_erase(StartAddressIntFlash);
      flash_write_data(DataBoot, StartAddressIntFlash, SIZE_RECORD_EXT_FLASH);

      StartAddressExtFlash += SIZE_RECORD_EXT_FLASH;
      StartAddressIntFlash += SIZE_RECORD_EXT_FLASH;
      CountPageData--;
   }
   FLASH_Lock();   //По окончанию записи блокируем флешпамять.

   return RET_OK;   // Операция обновления прошивки успешна.
}

//Записуем текущию прошивку во внешнию шлеш, так как не получилось скачать новую.
void WriteCurFirmware(void)
{
   uint32_t StartAddressExtFlash = ADDR_EXT_FLASH_NEW_FIRMWARE;
   uint32_t StartAddressIntFlash = START_ADDRESS_CODE_INT_FLASH;
   uint16_t CountPageData = (END_ADDRESS_CODE_INT_FLASH - (START_ADDRESS_CODE_INT_FLASH - 1)) / SIZE_RECORD_EXT_FLASH;

   //Отчищаем 28 субсектора внешней flash для записи прошивки.
   for (uint32_t i = StartAddressExtFlash; i < (StartAddressExtFlash + TOTAL_SIZE_FIRMWARE); i += SIZE_SUBSECTOR_FLASH)
   {
      LED_TOGGLE;
      FlashSubSectorEarse(i);
      IWDG_ReloadCounter();
   }

   while (CountPageData)
   {
      IWDG_ReloadCounter();   // Reload IWDG counter
      LED_TOGGLE;
      flash_read_data(DataBoot, StartAddressIntFlash, sizeof(DataBoot));
      EXT_FLASH_Write(DataBoot, StartAddressExtFlash, sizeof(DataBoot));
      StartAddressExtFlash += SIZE_RECORD_EXT_FLASH;
      StartAddressIntFlash += SIZE_RECORD_EXT_FLASH;
      CountPageData--;
   }
}

#ifdef FM4
void HARDWARE_Configuration(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;

   // Enable the GPIOs clocks
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC, ENABLE);

   // Enable the SPI2 clocks
   RCC_APB2PeriphClockCmd(SPI1_CLK, ENABLE);

   // PERIF REFERENCE

   // Configure ACCEL_SPI_CS_PIN pin: sEE_SPI Card CS pin //
   GPIO_InitStructure.GPIO_Pin = ACCEL_SPI_CS_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
   GPIO_Init(ACCEL_SPI_CS_GPIO_PORT, &GPIO_InitStructure);
   ACCEL_CS_OFF;

   GPIO_InitStructure.GPIO_Pin = FLASH_SPI_CS_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
   GPIO_Init(FLASH_SPI_CS_GPIO_PORT, &GPIO_InitStructure);
   FLASH_CS_OFF;

   // Init LEDS //
   GPIO_InitStructure.GPIO_Pin = LED_MCU;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_Init(LED_PORT, &GPIO_InitStructure);
   LED_OFF;

   // Configuring SPI1 //
   SPI1_LowLevel_Init();
}

#endif

#ifdef FM3
void HARDWARE_Configuration(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;

   // Enable the GPIOs clocks
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC, ENABLE);

   // Enable the SPI2 clocks
   RCC_APB2PeriphClockCmd(SPI2_CLK, ENABLE);

   /* PERIPH REF */
   GPIO_InitStructure.GPIO_Pin = PERIPH_REF_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_Init(PERIPH_REF_PORT, &GPIO_InitStructure);
   PERIPH_ALL_ON;

   // Configure ACCEL_SPI_CS_PIN pin: sEE_SPI Card CS pin //
   GPIO_InitStructure.GPIO_Pin = ACCEL_SPI_CS_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
   GPIO_Init(ACCEL_SPI_CS_GPIO_PORT, &GPIO_InitStructure);
   ACCEL_CS_OFF;

   GPIO_InitStructure.GPIO_Pin = FLASH_SPI_CS_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
   GPIO_Init(FLASH_SPI_CS_GPIO_PORT, &GPIO_InitStructure);
   FLASH_CS_OFF;

   // Init LEDS //
   GPIO_InitStructure.GPIO_Pin = LED_MCU;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_Init(LED_PORT, &GPIO_InitStructure);
   LED_OFF;

   // Init SPI2 //
   SPI2_LowLevel_Init();
}
#endif

void InitUsart(void)
{
   // Init Structure //
   GPIO_InitTypeDef GPIO_InitStructure;
   USART_InitTypeDef USART_InitStructure;

   // USART1 clock enable
   RCC_APB2PeriphClockCmd(USART1_CLK, ENABLE);

   // Enable the GPIOs clocks
   RCC_AHBPeriphClockCmd(USART1_PORT_CLK, ENABLE);

   // GPIO Config //
   GPIO_PinAFConfig(USART1_PORT, USART1_TX_SOURCE, USART1_AF);

   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_InitStructure.GPIO_Pin = USART1_TxPin;
   GPIO_Init(USART1_PORT, &GPIO_InitStructure);

   USART_InitStructure.USART_BaudRate = DBG_BAUDRATE;
   USART_InitStructure.USART_WordLength = USART_WordLength_8b;
   USART_InitStructure.USART_StopBits = USART_StopBits_1;
   USART_InitStructure.USART_Parity = USART_Parity_No;
   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
   USART_InitStructure.USART_Mode = USART_Mode_Tx;
   USART_Init(USART1, &USART_InitStructure);

   // Enable the USART1 //
   USART_Cmd(USART1, ENABLE);
}

void main(void)
{
   uint32_t AddresOffset = START_ADDRESS_CODE_INT_FLASH - FLASH_BASE;
   FIRMWARE_TYPE bootloader_true = NO_NEW_FIRMWARE;

   HARDWARE_Configuration();
   for (uint8_t i = 0; i < 5; i++)
   {
      delay(TIMEOUT);
   }

   /* Проверим наличие обновления прошивки */
   bootloader_true = BootloaderFirmwareTrue();
   if (bootloader_true != NO_NEW_FIRMWARE)
   {
      __enable_interrupt();
      // InitUsart();
      /* Отсутствие базовой прошивки на внешней flash (первое включение деваса) */
      if (bootloader_true == NO_BASE_FIRMWARE)
      {
         WriteCurFirmware();
      }
      /* Наличие новой прошивки на внешней flash */
      if (bootloader_true == UPDATE_FIRMWARE)
      {
         UpdateNewFirmware();
      }
   }
   __disable_interrupt();
   __disable_irq();
   NVIC_SetVectorTable(NVIC_VectTab_FLASH, AddresOffset);
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
   Jump_To_Application = (pFunction) * ((__IO uint32_t*)(START_ADDRESS_CODE_INT_FLASH + 0x04));   //Адресс вектора Reset
   __set_MSP(*(__IO uint32_t*)START_ADDRESS_CODE_INT_FLASH);   //Устанавливаем SP приложения

   /* START APPLICATION */
   Jump_To_Application();
}

#pragma optimize = none
static void delay(int iTimeout)
{
   for (int i = 0; i < iTimeout; i++)
   {
      IWDG_ReloadCounter();
      __NOP();
   }
}