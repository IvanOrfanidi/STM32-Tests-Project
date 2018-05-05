

#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H

// User functions //
#include "includes.h"

#define GPIO_HIGH(a, b) a->BSRR = b
#define GPIO_LOW(a, b) a->BRR = b
#define GPIO_TOGGLE(a, b) a->ODR ^= b
#define ON TRUE
#define OFF FALSE

// USB CONNECT
#define USB_M GPIO_Pin_13
#define PORT_USB_M GPIOC
#define PORT_USB_M_CLK RCC_APB2Periph_GPIOC

// LIGHT LED LCD
#define LED_LIGHT_LCD GPIO_Pin_6
#define PORT_LED_LIGHT_LCD GPIOA
#define PORT_LED_LIGHT_LCD_CLK RCC_APB2Periph_GPIOA

// LIGHT LED
#define LED GPIO_Pin_13
#define PORT_LED GPIOC
#define PORT_LED_CLK RCC_APB2Periph_GPIOC

#define BUT1 GPIO_Pin_1
#define BUT2 GPIO_Pin_0
#define BUT3 GPIO_Pin_15
#define PORT_BUT GPIOA

#define BUT4 GPIO_Pin_3
//#define PORT_LED          GPIOB

#define GET_BUT1 (!(GPIO_ReadInputDataBit(PORT_BUT, BUT1)))
#define GET_BUT2 (!(GPIO_ReadInputDataBit(PORT_BUT, BUT2)))
#define GET_BUT3 (!(GPIO_ReadInputDataBit(PORT_BUT, BUT3)))
#define GET_BUT4 (!(GPIO_ReadInputDataBit(GPIOB, BUT4)))

#define LED_ON GPIO_LOW(PORT_LED, LED);
#define LED_OFF GPIO_HIGH(PORT_LED, LED);
#define LED_TOGGLE GPIO_TOGGLE(PORT_LED, LED);

//// ----- GPS BAUDRATE ---- ////
#define GPS_BAUDRATE 9600
#define GSM_BAUDRATE 115200
#define DBG_BAUDRATE 19200
//**********************//

/// ----- NUM UART ---- ///
#define UART_ESP 3
#define UART_DBG 2
#define UART_GPS 1
//**********************//

#define ESP_BAUDRATE 115200

#define TIM3_PERIOD 1000
GLOBAL int16_t sTIMET1_CCR1_Val _EQU(700);

void InitGPIO(void);
void InitBKP(void);
void InitTIM3(void);
void InitIWDG(void);

#endif