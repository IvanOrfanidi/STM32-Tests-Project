#ifndef __PLATFORM_CONFIG_FM4_H_
#define __PLATFORM_CONFIG_FM4_H_

#ifndef BOOTLOADER
#include "includes.h"
#endif

#define __DEBUG__ 1
#define _CHECK_FUSES_ 0

#define DEV_VER "16"    // Индивидуальный номер каждого устройства.
/* 11 - iON Fm, 12 - Connect, 15 - FindMe3,  16 - FindMe4 */
#define HW_VER "01"    // Модификации каждого устройства(для FM911 это версия софта, что будет меняться).

#define NAME_FIRMWARE 1491295742    //Имя прошивки отображаемая на web

/* Макросы портов GPIO */
#define GPIO_HIGH(a, b) a->BSRRL = b
#define GPIO_LOW(a, b) a->BSRRH = b
#define GPIO_TOGGLE(a, b) a->ODR ^= b

#define MX25L4006E    // Микросхема FLASH памяти

#define TWO_SIMCARD 0    // Наличие двух СИМ карт

#define TEMPERATURE_ACCEL 1    // Измерение температуры акселерометром

#define USE_TEST_DEVICE 1    // Выход тестового режима на сторонний сервер отличный от 911

#define USE_LOG_DATA 0    // Отправка лога девайса на сервер iON

//// ----- USART BAUDRATE  ---- ////
#define GPS_BAUDRATE 9600
#define GSM_BAUDRATE 115200
#define DBG_BAUDRATE 57600

/// ----- NUM UART ---- ///
#define UART_GSM 2
#define UART_GPS 3
#define UART_DBG 1
//**********************//

#define MIN_TEMPERATUR_WORK -80

#define VCC_MCU (float)(3.30 * 1000)    // mV питание контроллера
#define BLEEDER_VIN (float)16.0
#define DIODE_DOWN 1000    // mV

//#define _PWR_CONTROL_ENABLE
#define _WDG_CONTROL_ENABLE

#define ADD_TIME_WAKEUP RTC_BKP_DR8
#define RESET_INFO_BACKUP_REGISTER RTC_BKP_DR9
#define DEVICE_SLEEP RTC_BKP_DR10
#define FLAG_SLEEP RTC_BKP_DR11
//**********************//

// Leds
#define LED_MCU GPIO_Pin_15
#define LED_PORT GPIOB

// SET GPS REFERENCE
#define GPS_REF_PIN GPIO_Pin_4
#define GPS_REF_PORT GPIOA

// SET GSM REFERENCE
#define GSM_REF_PIN GPIO_Pin_9
#define GSM_REF_PORT GPIOB

// KEY FOR SIM800
#define GSM_PWR_KEY_PIN GPIO_Pin_11
#define GSM_PWR_KEY_PIN_PORT GPIOA

// ADC
#define VIN_Meas_PIN GPIO_Pin_0
#define ADC_PORT GPIOB
#define VIN_ADC_CHNL_NUM ADC_Channel_8

// SPI DEVICE //
// CHIP SELECT FLASH //
#define FLASH_SPI_CS_PIN GPIO_Pin_12
#define FLASH_SPI_CS_GPIO_PORT GPIOB
#define FLASH_SPI_CS_GPIO_CLK RCC_AHBPeriph_GPIOB

// CHIP SELECT ACCEL //
#define ACCEL_SPI_CS_PIN GPIO_Pin_1
#define ACCEL_SPI_CS_GPIO_PORT GPIOB
#define ACCEL_SPI_CS_GPIO_CLK RCC_AHBPeriph_GPIOB
//**********************************************************//

// INTERRUPT ACCEL //
#define ACCEL_INT_PIN GPIO_Pin_0
#define ACCEL_INT_GPIO_PORT GPIOA
#define ACCEL_INT_GPIO_CLK RCC_AHBPeriph_GPIOA

// SIM PIN ON/OFF
#define SIM_ON_PIN GPIO_Pin_15
#define SIM_ON_PORT GPIOA

/* SPI */
/* SPI1 DEVICE */
#define SPI1_CLK RCC_APB2Periph_SPI1    // RCC_APB2Periph_SPI1
#define SPI1_SCK_PIN GPIO_Pin_5         /* PB.13 */
#define SPI1_SCK_GPIO_PORT GPIOA        /* GPIOB */
#define SPI1_SCK_GPIO_CLK RCC_AHBPeriph_GPIOA
#define SPI1_SCK_SOURCE GPIO_PinSource5
#define SPI1_SCK_AF GPIO_AF_SPI1
#define SPI1_MISO_PIN GPIO_Pin_6  /* PB.14 */
#define SPI1_MISO_GPIO_PORT GPIOA /* GPIOB */
#define SPI1_MISO_GPIO_CLK RCC_AHBPeriph_GPIOA
#define SPI1_MISO_SOURCE GPIO_PinSource6
#define SPI1_MISO_AF GPIO_AF_SPI1
#define SPI1_MOSI_PIN GPIO_Pin_7  /* PB.15 */
#define SPI1_MOSI_GPIO_PORT GPIOA /* GPIOB */
#define SPI1_MOSI_GPIO_CLK RCC_AHBPeriph_GPIOA
#define SPI1_MOSI_SOURCE GPIO_PinSource7
#define SPI1_MOSI_AF GPIO_AF_SPI1
/********************/

/* SPI2 DEVICE */
#define SPI2_CLK RCC_APB1Periph_SPI2 /* RCC_APB1Periph_SPI2 */
#define SPI2_SCK_PIN GPIO_Pin_13     /* PB.13 */
#define SPI2_SCK_GPIO_PORT GPIOB     /* GPIOB */
#define SPI2_SCK_GPIO_CLK RCC_AHBPeriph_GPIOB
#define SPI2_SCK_SOURCE GPIO_PinSource13
#define SPI2_SCK_AF GPIO_AF_SPI2
#define SPI2_MISO_PIN GPIO_Pin_14 /* PB.14 */
#define SPI2_MISO_GPIO_PORT GPIOB /* GPIOB */
#define SPI2_MISO_GPIO_CLK RCC_AHBPeriph_GPIOB
#define SPI2_MISO_SOURCE GPIO_PinSource14
#define SPI2_MISO_AF GPIO_AF_SPI2
#define SPI2_MOSI_PIN GPIO_Pin_15 /* PB.15 */
#define SPI2_MOSI_GPIO_PORT GPIOB /* GPIOB */
#define SPI2_MOSI_GPIO_CLK RCC_AHBPeriph_GPIOB
#define SPI2_MOSI_SOURCE GPIO_PinSource15
#define SPI2_MOSI_AF GPIO_AF_SPI2
/********************/

#define SIM_ON SimPwrOn()
#define SIM_OFF SimPwrOff()

#define LED_ON GPIO_HIGH(LED_PORT, LED_MCU)
#define LED_OFF GPIO_LOW(LED_PORT, LED_MCU)
#define LED_TOGGLE GPIO_TOGGLE(LED_PORT, LED_MCU)

#define FLASH_CS_OFF \
    GPIO_HIGH(FLASH_SPI_CS_GPIO_PORT, FLASH_SPI_CS_PIN); \
    SPI1_CS_DELAY
#define FLASH_CS_ON \
    GPIO_LOW(FLASH_SPI_CS_GPIO_PORT, FLASH_SPI_CS_PIN); \
    SPI1_CS_DELAY

#define ACCEL_CS_OFF \
    GPIO_HIGH(ACCEL_SPI_CS_GPIO_PORT, ACCEL_SPI_CS_PIN); \
    SPI1_CS_DELAY
#define ACCEL_CS_ON \
    GPIO_LOW(ACCEL_SPI_CS_GPIO_PORT, ACCEL_SPI_CS_PIN); \
    SPI1_CS_DELAY

#define GSM_STATUS_ON \
    GPIO_ReadInputDataBit(GSM_REF_PORT, GSM_REF_PIN)    // Проверяем старт GSM модуля по напряжению на его пине.

#define PWR_KEY_PULL_DOWN GPIO_LOW(GSM_PWR_KEY_PIN_PORT, GSM_PWR_KEY_PIN)
#define PWR_KEY_PULL_UP GPIO_HIGH(GSM_PWR_KEY_PIN_PORT, GSM_PWR_KEY_PIN)

/* -=USARTS=- */
#define USART1_Tx_DMA_Channel DMA1_Channel4
#define USART1_Rx_DMA_Channel DMA1_Channel5
#define USART1_DR_Base USART1_BASE + 0x04

#define USART2_Tx_DMA_Channel DMA1_Channel7
#define USART2_Rx_DMA_Channel DMA1_Channel6
#define USART2_DR_Base USART2_BASE + 0x04

#define USART3_Tx_DMA_Channel DMA1_Channel2
#define USART3_Rx_DMA_Channel DMA1_Channel3
#define USART3_DR_Base USART3_BASE + 0x04
//********************************//

/* USART1 */
#define USART1_PORT GPIOA
#define USART1_CLK RCC_APB2Periph_USART1
#define USART1_PORT_CLK RCC_AHBPeriph_GPIOA
#define USART1_TxPin GPIO_Pin_9
#define USART1_RxPin GPIO_Pin_10
#define USART1_TX_SOURCE GPIO_PinSource9
#define USART1_RX_SOURCE GPIO_PinSource10
#define USART1_AF GPIO_AF_USART1

/* USART2 */
#define USART2_PORT GPIOA
#define USART2_CLK RCC_APB1Periph_USART2
#define USART2_PORT_CLK RCC_AHBPeriph_GPIOA
#define USART2_TxPin GPIO_Pin_2
#define USART2_RxPin GPIO_Pin_3
#define USART2_TX_SOURCE GPIO_PinSource2
#define USART2_RX_SOURCE GPIO_PinSource3
#define USART2_AF GPIO_AF_USART2

/* USART3 */
#define USART3_PORT GPIOB
#define USART3_CLK RCC_APB1Periph_USART3
#define USART3_PORT_CLK RCC_AHBPeriph_GPIOB
#define USART3_TxPin GPIO_Pin_10
#define USART3_RxPin GPIO_Pin_11
#define USART3_TX_SOURCE GPIO_PinSource10
#define USART3_RX_SOURCE GPIO_PinSource11
#define USART3_AF GPIO_AF_USART3
/****************************************************/

#define ReStartDmaGsmUsart() ReStartDmaUsart2()
#define ReStartDmaDbgUsart() ReStartDmaUsart1()
#define ReStartDmaGpsUsart() ReStartDmaUsart3()

#define SLEEP_MS_10000 10000
#define SLEEP_MS_5000 5000
#define SLEEP_MS_4000 4000
#define SLEEP_MS_3000 3000
#define SLEEP_MS_2500 2500
#define SLEEP_MS_2000 2000
#define SLEEP_MS_1500 1500
#define SLEEP_MS_1000 1000
#define SLEEP_MS_800 800
#define SLEEP_MS_500 500
#define SLEEP_MS_400 400
#define SLEEP_MS_250 250
#define SLEEP_MS_200 200
#define SLEEP_MS_100 100
#define SLEEP_MS_50 50
#define SLEEP_MS_10 10
#define SLEEP_MS_5 5
#define SLEEP_MS_1 1

void RCC_Configuration(void);
void GPIO_Configuration(void);
#ifndef BOOTLOADER
void GPS_Reference(VALUE prm);
#endif
void GSM_Reference(_Bool val);
void IWDGInit(void);
void CheckFuses(void);
uint32_t ReadNameNewFirmware(void);
uint32_t ReadNameBaseFirmware(void);
void SimPwrOn(void);
void SimPwrOff(void);
#endif