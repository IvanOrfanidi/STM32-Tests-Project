

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
#define LED GPIO_Pin_13
#define PORT_LED GPIOC
#define PORT_LED_CLK RCC_APB2Periph_GPIOC

#define LED_ON GPIO_HIGH(PORT_LED, LED);
#define LED_OFF GPIO_LOW(PORT_LED, LED);
#define LED_TOGGLE GPIO_TOGGLE(PORT_LED, LED);

//// ---- GPS ---- ////
//#define GPS_MODULE_IT520
#define GPS_MODULE_UBLOX

#ifdef GPS_MODULE_IT520
#define GPS_PACKET_LEN 1
#endif

#ifdef GPS_MODULE_UBLOX
#define GPS_PACKET_LEN 0
#endif

//// ----- GPS BAUDRATE ---- ////
#ifdef GPS_MODULE_IT520
#define GPS_BAUDRATE 115200
#endif

#ifdef GPS_MODULE_UBLOX
#define GPS_BAUDRATE 9600
#endif

#define GSM_BAUDRATE 115200
#define DBG_BAUDRATE 19200
//**********************//

/// ----- NUM UART ---- ///
//#define UART_GPS         3
#define UART_DBG 2
//**********************//

void InitGPIO(void);
void InitBKP(void);

#endif