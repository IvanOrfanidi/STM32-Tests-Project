#ifndef _DRF_H_
#define _DRF_H_

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>

#define DRF_EN_DELAY \
    for(int _ii = 0; _ii < 100; _ii++) \
        ;

#define DRF_EN_PIN GPIO_Pin_4
#define DRF_EN_PORT GPIOA
#define DRF_EN_GPIO_CLK RCC_APB2Periph_GPIOA

#define DRF_ENABLE \
    GPIO_SetBits(DRF_EN_PORT, DRF_EN_PIN); \
    for(uint16_t _ii = 0; _ii < 300; _ii++) \
        ;
#define DRF_DISABLE \
    GPIO_ResetBits(DRF_EN_PORT, DRF_EN_PIN); \
    for(uint16_t _ii = 0; _ii < 10; _ii++) \
        ;

#define DRF_SET_PIN GPIO_Pin_1
#define DRF_SET_PORT GPIOA
#define DRF_SET_GPIO_CLK RCC_APB2Periph_GPIOA

#define DRF_SET_ENABLE \
    GPIO_ResetBits(DRF_SET_PORT, DRF_SET_PIN); \
    for(uint16_t _ii = 0; _ii < 100; _ii++) \
        ;
#define DRF_SET_DISABLE \
    GPIO_SetBits(DRF_SET_PORT, DRF_SET_PIN); \
    for(uint16_t _ii = 0; _ii < 100; _ii++) \
        ;

#define _AUX_INTERRUPT_USE 0
#define DRF_AUX_PIN GPIO_Pin_0
#define DRF_AUX_PORT GPIOA
#define DRF_AUX_GPIO_CLK RCC_APB2Periph_GPIOA

#if _AUX_INTERRUPT_USE == 1
#define EXT_AUX_Line EXTI_Line0
#define EXT_AUX_IRQn EXTI0_IRQn
#endif

#define MAX_FREQUENCY 455000
#define MIN_FREQUENCY 418000

#define DRF_SETTING_BAUDRATE 9600

typedef enum {
    ERR_RESP = -2,
    ERR_TIMEOUT = -1,
    OK_RESP = 0,
} RESP;

typedef enum {
    DRfsk_1200bps = 0x30,
    DRfsk_2400bps = 0x31,
    DRfsk_4800bps = 0x32,
    DRfsk_9600bps = 0x33,
    DRfsk_19200bps = 0x34
} DRF_DR_FSK;

typedef enum {
    Pout_11dBm = 0x30,
    Pout_12dBm = 0x31,
    Pout_13dBm = 0x32,
    Pout_14dBm = 0x33,
    Pout_15dBm = 0x34,
    Pout_16dBm = 0x35,
    Pout_17dBm = 0x36,
    Pout_18dBm = 0x37,
    Pout_19dBm = 0x38,
    Pout_20dBm = 0x39
} DRF_RF_POWER;

typedef enum {
    DRin_1200bps = 0x30,
    DRin_2400bps = 0x31,
    DRin_4800bps = 0x32,
    DRin_9600bps = 0x33,
    DRin_19200bps = 0x34,
    DRin_38400bps = 0x35,
    DRin_57600bps = 0x36
} DRF_DR_IN;

typedef enum {
    NO_PARITY = 0x30,
    EVEN_PARITY = 0x31,
    ODD_PARITY = 0x32
} DRF_PARITY;

typedef __packed struct
{
    uint32_t uiFreq;
    DRF_DR_FSK eDRfsk;
    DRF_RF_POWER ecPout;
    DRF_DR_IN eDRin;
    DRF_PARITY eParity;
} TDrf_Settings;

void InitGpioDRF(void);
int GetConfigDRF(TDrf_Settings* pstDrfSettings, uint32_t uiBaudRate);
int SetConfigDRF(TDrf_Settings* pstDrfSettings, uint32_t uiBaudRate);
void SetRadioEnable(_Bool bEnabDrf);

#endif