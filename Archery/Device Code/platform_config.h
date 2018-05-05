

#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H

// User functions //
#include "includes.h"

#define GPIO_HIGH(a, b) a->BSRR = b
#define GPIO_LOW(a, b) a->BRR = b
#define GPIO_TOGGLE(a, b) a->ODR ^= b
#define ON TRUE
#define OFF FALSE

// LIGHT LED
#define LED GPIO_Pin_13
#define PORT_LED ((GPIO_TypeDef*)GPIOC_BASE)
#define PORT_LED_CLK RCC_APB2Periph_GPIOC

#define LED_ON GPIO_LOW(PORT_LED, LED);
#define LED_OFF GPIO_HIGH(PORT_LED, LED);
#define LED_TOGGLE GPIO_TOGGLE(PORT_LED, LED);

// BUTTON
#define BUT GPIO_Pin_2
#define PORT_BUT ((GPIO_TypeDef*)GPIOA_BASE)
#define PORT_BUT_CLK RCC_APB2Periph_GPIOA

#define GET_BUT (!(GPIO_ReadInputDataBit(PORT_BUT, BUT)))

// BUZZER
#define BUZ_PIN GPIO_Pin_6
#define PORT_BUZ GPIOB
#define PORT_BUZ_CLK RCC_APB2Periph_GPIOB

#define CHARGING_COMPLETED_GPIO_PIN GPIO_Pin_1
#define CHARGING_POWER_GPIO_PIN GPIO_Pin_0
#define CHARGING_GPIO_PORT ((GPIO_TypeDef*)GPIOA_BASE)
#define PORT_CHARGING_CLK RCC_APB2Periph_GPIOA

#define MIN_BRIGHTNESS 3
#define MAX_BRIGHTNESS 7

#define DBG_BAUDRATE 115200
//**********************//

/// ----- NUM UART ---- ///
#define UART_DBG 1
//**********************//

#define TIM_BUZ_PERIOD 1000
GLOBAL uint16_t usTIM_BUZ_CCR1_Val _EQU(500);

void InitGPIO(void);
void InitBKP(void);
void SleepDevice(void);
void InitTIM_BUZ(void);

#endif