
#ifndef BOOTLOADER
#   include "includes.h"
#else
#   include "stm32l1xx.h"
#   include "arm_comm.h"

#   include <stdio.h>
#   include <string.h>
#   include <ctype.h>
#   include <stdlib.h>
#   include <limits.h>
#   include <intrinsics.h>
#   include <assert.h>

#   include "EXT_FLASH.h"
#   include "INT_FLASH.h"
#   include "SPI.h"
#   include "crc16.h"
#   include "eeprom.h"
#endif

// Тактирование портов //
void RCC_Configuration(void)
{
   // Enable the GPIOs clocks
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC, ENABLE);

#ifdef __USART_H
   // USART2, USART3
   RCC_APB1PeriphClockCmd(USART2_CLK | USART3_CLK, ENABLE);
   // USART1, ADC1 clock enable
   RCC_APB2PeriphClockCmd(USART1_CLK, ENABLE);
#endif

#ifdef _SPI_H_
   // Enable SPI1 //
   RCC_APB2PeriphClockCmd(SPI1_CLK, ENABLE);
#endif
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
}

// Конфигурация портов ввода/вывода //
void GPIO_Configuration(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;

   /* Init LEDS */
   GPIO_InitStructure.GPIO_Pin = LED_MCU;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_Init(LED_PORT, &GPIO_InitStructure);
   LED_OFF;

#ifdef FM3
   /* PERIPH REF */
   GPIO_InitStructure.GPIO_Pin = PERIPH_REF_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_Init(PERIPH_REF_PORT, &GPIO_InitStructure);
   PERIPH_ALL_ON;
#endif

   /* GSM REFERENCE */
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_InitStructure.GPIO_Pin = GSM_REF_PIN;
   GPIO_Init(GSM_REF_PORT, &GPIO_InitStructure);
   GPIO_LOW(GSM_REF_PORT, GSM_REF_PIN);

   /* GSM DTR */
#if FM3
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_InitStructure.GPIO_Pin = GSM_DTR_PIN;
   GPIO_Init(GSM_DTR_PORT, &GPIO_InitStructure);
   GSM_UART_START;
#endif

   /* GPS REFERENCE */
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_InitStructure.GPIO_Pin = GPS_REF_PIN;
   GPIO_Init(GPS_REF_PORT, &GPIO_InitStructure);
   GPIO_HIGH(GPS_REF_PORT, GPS_REF_PIN);

   /* Output PWR KEY SIM800 */
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_InitStructure.GPIO_Pin = GSM_PWR_KEY_PIN;
   GPIO_Init(GSM_PWR_KEY_PIN_PORT, &GPIO_InitStructure);
   PWR_KEY_PULL_UP;

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

   /* Configure SIM PIN ON/OFF */
#ifdef FM4
   GPIO_InitStructure.GPIO_Pin = SIM_ON_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_Init(SIM_ON_PORT, &GPIO_InitStructure);
#endif
   SIM_OFF;
}

void GPS_Reference(VALUE prm)
{
   if (prm == ON)
   {
#ifdef FM4
      GPIO_HIGH(GPS_REF_PORT, GPS_REF_PIN);
#else
      GPIO_LOW(GPS_REF_PORT, GPS_REF_PIN);
#endif
   }
   else
   {
#ifdef FM4
      GPIO_LOW(GPS_REF_PORT, GPS_REF_PIN);
#else
      GPIO_HIGH(GPS_REF_PORT, GPS_REF_PIN);
#endif
   }
}

void GSM_Reference(_Bool val)
{
   if (val)
   {
      DPS("-D_GSM PWR ON-\r\n");
      GPIO_HIGH(GSM_REF_PORT, GSM_REF_PIN);
   }
   else
   {
      DPS("-D_GSM PWR OFF-\r\n");
      GPIO_LOW(GSM_REF_PORT, GSM_REF_PIN);
   }
}

void IWDGInit(void)
{
#ifdef _WDG_CONTROL_ENABLE
   /* Enable the LSI OSC */
   RCC_LSICmd(ENABLE);

   /* Wait till LSI is ready */
   while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
      ;

   /* Enable write access to IWDG_PR and IWDG_RLR registers */
   IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

   /* IWDG counter clock: LSI/256 */
   IWDG_SetPrescaler(IWDG_Prescaler_32);
   IWDG_SetReload(0x0FFF);   // This parameter must be a number between 0 and 0x0FFF.

   /* Reload IWDG counter */
   IWDG_ReloadCounter();

   /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
   IWDG_Enable();
#endif
}

uint32_t ReadNameNewFirmware(void)
{
   uint8_t DataBoot[256];
   uint32_t uiNameNewFirmware = 0;   //Имя новой прошивки во внешней flash.

   char* p1 = (char*)&uiNameNewFirmware;
   char* p2 = (char*)&DataBoot[240];
   uint32_t uiAdd = (ADDR_EXT_FLASH_NEW_FIRMWARE + TOTAL_SIZE_FIRMWARE) - 256;
   EXT_FLASH_Read(DataBoot, uiAdd, 256);
   for (uint8_t i = 0; i < sizeof(uiNameNewFirmware); i++)
   {
      p1[i] = p2[i];
   }

   return uiNameNewFirmware;
}
/*
uint32_t ReadNameBaseFirmware(void)
{
  char TempBuf[256];
  uint32_t uiNameBaseFirmware = 0;       //Имя новой прошивки во внешней flash.
  
  memset(TempBuf, 0, sizeof(TempBuf));
  char *p1 = (char *)&uiNameBaseFirmware;
  char *p2 = (char *)&TempBuf[240];
  
  if ( osKernelRunning() ) {
    xSemaphoreTake(xBinSemFLASH, portMAX_DELAY);
  }
  EXT_FLASH_Read((uint8_t *)TempBuf, ADDR_EXT_FLASH_BASE_FIRMWARE + 256*487, 256);
  
  if ( osKernelRunning() ) {
    xSemaphoreGive(xBinSemFLASH);
  }
  for(uint8_t i=0; i<sizeof(uiNameBaseFirmware); i++){
     p1[i] = p2[i];
  }

  return uiNameBaseFirmware;
}
*/

void SimPwrOn(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;
   /* Configure SIM PIN ON */
   GPIO_InitStructure.GPIO_Pin = SIM_ON_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
   GPIO_Init(SIM_ON_PORT, &GPIO_InitStructure);
   GPIO_LOW(SIM_ON_PORT, SIM_ON_PIN);
}

void SimPwrOff(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;
   /* Configure SIM PIN OFF */
   GPIO_InitStructure.GPIO_Pin = SIM_ON_PIN;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_Init(SIM_ON_PORT, &GPIO_InitStructure);
}

/* Отключаем оптимизацию кода для функции защиты чипа */
#pragma optimize = none
/* Включает защиту чипа от чтения прошивки с flash */
void CheckFuses(void)
{
#ifndef _CHECK_FUSES_
#   define _CHECK_FUSES_ 0
#endif
#if (_CHECK_FUSES_ == 1)
   if (FLASH_OB_GetRDP() == RESET)
   {
      for (uint32_t i = 0; i < 100000; i++)
      {
         /* Reload IWDG counter */
         IWDG_ReloadCounter();
         __NOP();
      }

      FLASH_OB_Unlock();   // Открывает возможность блокировать доступ байт.
      FLASH_OB_BORConfig(OB_BOR_LEVEL2);
      FLASH_OB_RDPConfig(OB_RDP_Level_1);
      FLASH_OB_Launch();   // Запустите опцию загрузки байт.
      FLASH_OB_Lock();
   }
#endif
}

_Bool gsm_sleep = TRUE;   //показывает что gsm находится в сне
_Bool GetSleepGsm(void)
{
   return gsm_sleep;
}

void SetSleepGsm(_Bool val)
{
   gsm_sleep = val;
}

int GSM_State(VALUE val)
{
#define TIMEOUT_POWER_GSM 1000

   if (val == ON)
   {
      memset(g_asCmdBuf, 0, sizeof(g_asCmdBuf));
      memset(g_asRxBuf, 0, sizeof(g_asRxBuf));
      memset(g_aucTxBufferUSART2, 0, GSM_TX_BUFFER_SIZE);
      memset(g_aucRxBufferUSART2, 0, GSM_RX_BUFFER_SIZE);

      InitUSART(UART_GSM, GSM_BAUDRATE);
      InitDMA(UART_GSM);

      ReStartDmaGsmUsart();   //Перезапускаем DMA, так навсякий случай.

      GSM_Reference(val);

      portTickType xLastWakeTimerDelay = xTaskGetTickCount();
      vTaskDelayUntil(&xLastWakeTimerDelay, (SLEEP_MS_500 / portTICK_RATE_MS));
      PWR_KEY_PULL_UP;
      xLastWakeTimerDelay = xTaskGetTickCount();
      vTaskDelayUntil(&xLastWakeTimerDelay, (SLEEP_MS_500 / portTICK_RATE_MS));
      PWR_KEY_PULL_DOWN;
      SetSleepGsm(FALSE);

      loop(TIMEOUT_POWER_GSM);
      return RET_OK;
   }
   else if (val == OFF)
   {
      GsmModemCmdOff();
      DeInitUSART(UART_GSM);
      GSM_Reference(val);
      SetSleepGsm(TRUE);
      SetCSQ(0);

      loop(TIMEOUT_POWER_GSM);
      return RET_OK;
   }
   else if (val == SLEEP)
   {
      SetGsmFunctional(MIN_FUNCTIONALITI);
      SetSleepGsm(TRUE);
   }
   else if (val == WAKE_UP)
   {
      SetGsmFunctional(FULL_FUNCTIONALITI);
      SetSleepGsm(FALSE);
   }

   return ERR_POWER;
}