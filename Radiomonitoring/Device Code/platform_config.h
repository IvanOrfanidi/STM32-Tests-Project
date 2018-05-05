

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

// LIGHT LED
#define LED_PIN GPIO_Pin_13
#define PORT_LED GPIOC
#define PORT_LED_CLK RCC_APB2Periph_GPIOC

#define LED_ON GPIO_LOW(PORT_LED, LED_PIN);
#define LED_OFF GPIO_HIGH(PORT_LED, LED_PIN);
#define LED_TOGGLE GPIO_TOGGLE(PORT_LED, LED_PIN);

// BUZZER
#define BUZ_PIN GPIO_Pin_14
#define PORT_BUZ GPIOC
#define PORT_BUZ_CLK RCC_APB2Periph_GPIOC

#define BUZ_ON GPIO_HIGH(PORT_BUZ, BUZ_PIN);
#define BUZ_OFF GPIO_LOW(PORT_BUZ, BUZ_PIN);

// BUTTON
#define BUT_PIN GPIO_Pin_15
#define PORT_BUT GPIOC
#define PORT_BUT_CLK RCC_APB2Periph_GPIOC

#define GET_BUT (!(GPIO_ReadInputDataBit(PORT_BUT, BUT_PIN)))

#define DBG_BAUDRATE 9600
#define DRF_BAUDRATE 2400
//**********************//

/// ----- NUM UART ---- ///
#define UART_DBG 1
#define UART_DRF 2
//**********************//

#define DRF_RX_BUFFER_SIZE RX_BUFFER_SIZE2

void InitGPIO(void);
void InitBKP(void);
void InitIWDG(void);
void SleepMasterDevice(void);
void SleepSlaveDevice(void);

#endif