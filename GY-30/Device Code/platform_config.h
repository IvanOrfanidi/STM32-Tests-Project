

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
#define LED GPIO_Pin_13
#define PORT_LED GPIOC
#define PORT_LED_CLK RCC_APB2Periph_GPIOC

#define LED_ON GPIO_LOW(PORT_LED, LED);
#define LED_OFF GPIO_HIGH(PORT_LED, LED);
#define LED_TOGGLE GPIO_TOGGLE(PORT_LED, LED);

#define DBG_BAUDRATE 19200
//**********************//

/// ----- NUM UART ---- ///
#define UART_DBG 1
//**********************//

#define TIM3_PERIOD 1000
GLOBAL uint16_t usTIMET3_CCR1_Val _EQU(500);

#define TIM4_PERIOD 1000
GLOBAL uint16_t usTIMET4_CCR1_Val _EQU(500);

void InitGPIO(void);
void InitBKP(void);
void InitTIM3(void);
void InitTIM4(void);
void InitIWDG(void);

#endif