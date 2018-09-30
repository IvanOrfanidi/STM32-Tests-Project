

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
#define LED GPIO_Pin_1
#define PORT_LED GPIOA
#define PORT_LED_CLK RCC_APB2Periph_GPIOA

#define LED_ON GPIO_LOW(PORT_LED, LED);
#define LED_OFF GPIO_HIGH(PORT_LED, LED);
#define LED_TOGGLE GPIO_TOGGLE(PORT_LED, LED);

#define DBG_BAUDRATE 19200
//**********************//

/// ----- NUM UART ---- ///
#define UART_DBG 1
//**********************//

void InitGPIO(void);
void InitBKP(void);
void InitIWDG(void);

#endif