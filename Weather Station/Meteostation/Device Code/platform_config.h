

#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H

// User functions //
#include "includes.h"

#define GPIO_HIGH(a, b) a->BSRR = b
#define GPIO_LOW(a, b) a->BRR = b
#define GPIO_TOGGLE(a, b) a->ODR ^= b
#define ON TRUE
#define OFF FALSE

// BUTTON
#define BUT_S1 GPIO_Pin_0
#define BUT_S2 GPIO_Pin_1
#define PORT_BUT GPIOC
#define PORT_BUT_CLK RCC_APB2Periph_GPIOC

// USB CONNECT
#define USB_M GPIO_Pin_13
#define PORT_USB_M GPIOC
#define PORT_USB_M_CLK RCC_APB2Periph_GPIOC

// LIGHT LED LCD
#define LED_LIGHT_LCD GPIO_Pin_6
#define PORT_LED_LIGHT_LCD GPIOB
#define PORT_LED_LIGHT_LCD_CLK RCC_APB2Periph_GPIOB

#define BUZ GPIO_Pin_1
#define PORT_BUZ GPIOA
#define PORT_BUZ_CLK RCC_APB2Periph_GPIOC

#define LED_D1_ON GPIO_HIGH(PORT_LED, LED_D1);
#define LED_D1_OFF GPIO_LOW(PORT_LED, LED_D1);

#define LED_D2_ON GPIO_HIGH(PORT_LED, LED_D2);
#define LED_D2_OFF GPIO_LOW(PORT_LED, LED_D2);

#define BUZ_ON GPIO_HIGH(PORT_BUZ, BUZ);
#define BUZ_OFF GPIO_LOW(PORT_BUZ, BUZ);

#define LED_LIGHT_LCD_ON GPIO_HIGH(PORT_LED_LIGHT_LCD, LED_LIGHT_LCD);
#define LED_LIGHT_LCD_OFF GPIO_LOW(PORT_LED_LIGHT_LCD, LED_LIGHT_LCD);

//// ---- GPS ---- ////
//#define GPS_MODULE_IT520
#define GPS_MODULE_UBLOX

#ifdef GPS_MODULE_IT520
#   define GPS_PACKET_LEN 1
#endif

#ifdef GPS_MODULE_UBLOX
#   define GPS_PACKET_LEN 0
#endif

//// ----- GPS BAUDRATE ---- ////
#ifdef GPS_MODULE_IT520
#   define GPS_BAUDRATE 115200
#endif

#ifdef GPS_MODULE_UBLOX
#   define GPS_BAUDRATE 9600
#endif

#define GSM_BAUDRATE 115200
#define DBG_BAUDRATE 19200
//**********************//

/// ----- NUM UART ---- ///
//#define UART_GPS         3
#define UART_DBG 1
//**********************//

void InitGPIO(void);
void InitBKP(void);

#endif