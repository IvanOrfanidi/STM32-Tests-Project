#ifndef _BH1750_H_
#define _BH1750_H_

#include "includes.h"

#define _I2C_PORT_BH1750_ 2

#if _I2C_PORT_BH1750_ == 1
#define BH1750_I2C I2C1
#define BH1750_I2C_RCC_Periph RCC_APB1Periph_I2C1
#define BH1750_I2C_Port GPIOB
#define BH1750_I2C_SCL_Pin GPIO_Pin_6
#define BH1750_I2C_SDA_Pin GPIO_Pin_7
#define BH1750_I2C_RCC_Port RCC_APB2Periph_GPIOB
#elif _I2C_PORT_BH1750_ == 2
#define BH1750_I2C I2C2
#define BH1750_I2C_RCC_Periph RCC_APB1Periph_I2C2
#define BH1750_I2C_Port GPIOB
#define BH1750_I2C_SCL_Pin GPIO_Pin_10
#define BH1750_I2C_SDA_Pin GPIO_Pin_11
#define BH1750_I2C_RCC_Port RCC_APB2Periph_GPIOB
#endif

#define BH1750_I2C_Speed 100000

#define BH1750_ADDRESS 0x46    // 0x23 // this device only has one address

// No active state
#define BH1750_POWER_DOWN 0x00

// Wating for measurment command
#define BH1750_POWER_ON 0x01

// Reset data register value - not accepted in POWER_DOWN mode
#define BH1750_RESET 0x07

// Start measurement at 1lx resolution. Measurement time is approx 120ms.
#define BH1750_CONTINUOUS_HIGH_RES_MODE 0x10

// Start measurement at 0.5lx resolution. Measurement time is approx 120ms.
#define BH1750_CONTINUOUS_HIGH_RES_MODE_2 0x11

// Start measurement at 4lx resolution. Measurement time is approx 16ms.
#define BH1750_CONTINUOUS_LOW_RES_MODE 0x13

// Start measurement at 1lx resolution. Measurement time is approx 120ms.
// Device is automatically set to Power Down after measurement.
#define BH1750_ONE_TIME_HIGH_RES_MODE 0x20

// Start measurement at 0.5lx resolution. Measurement time is approx 120ms.
// Device is automatically set to Power Down after measurement.
#define BH1750_ONE_TIME_HIGH_RES_MODE_2 0x21

// Start measurement at 1lx resolution. Measurement time is approx 120ms.
// Device is automatically set to Power Down after measurement.
#define BH1750_ONE_TIME_LOW_RES_MODE 0x23

#define DEF_TIMEOUT_I2C 300

void BH1750_I2C_Init(void);
void BH1750_Init(void);
uint32_t BH1750_Read(void);

#endif