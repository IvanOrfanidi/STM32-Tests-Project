

#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H

#include "includes.h"

#define GPIO_HIGH(a,b) 		GPIO_SetBits(a,b)       //a->BSRR = b
#define GPIO_LOW(a,b)		GPIO_ResetBits(a,b)     //a->BRR = b
#define GPIO_TOGGLE(a,b) 	a->ODR ^= b 
#define ON              TRUE
#define OFF             FALSE

#define BUT_S1                   GPIO_Pin_5
#define BUT_S2                   GPIO_Pin_4
#define BUT_S3                   GPIO_Pin_3
#define BUT_S4                   GPIO_Pin_2
#define PORT_BUT               ((GPIO_TypeDef *) GPIOE_BASE)


#define LED_D1                   GPIO_Pin_6
#define LED_D2                   GPIO_Pin_7
#define LED_D3                   GPIO_Pin_13
#define LED_D4                   GPIO_Pin_6
#define PORT_LED               ((GPIO_TypeDef *) GPIOC_BASE)

#define BUZ_SP1                  GPIO_Pin_0
#define PORT_BUZ               ((GPIO_TypeDef *) GPIOE_BASE)

void InitPIO(void);
void RTC_Configuration(void);
void InitBKP(void);
#endif