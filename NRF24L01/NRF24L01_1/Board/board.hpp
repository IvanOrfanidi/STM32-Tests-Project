/**
 ******************************************************************************
 * @file    boadr.hpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    28-March-2018
 * @brief   This file contains all the methods prototypes for the RTC
 *          firmware library.
 ******************************************************************************
 * @attention
 *
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BOARD_HPP
#define __BOARD_HPP

#include <stdint.h>
#include "stm32f10x_gpio.h"

/// LIGHT LED
#define LED_PIN GPIO_Pin_1
#define PORT_LED GPIOA
#define PORT_LED_CLK RCC_APB2Periph_GPIOA

#ifdef __cplusplus

/*
 * @brief Class Board
 */
class Board {
  public:
    static void InitLed();    ///< Initialisation Led

    static void InitBKP();    ///< Initialisation Backup

    static void InitIWDG();    ///< Initialisation Watchdog timer

    static void LedOn();    ///< Enable Led

    static void LedOff();    ///< Disable Led

    static void GpioClock(const GPIO_TypeDef*, FunctionalState);    ///< Enable/Disable Clock Port
};

/*
 * @brief Extern interrupt
 */
extern "C" {
}
#endif    //__cplusplus

#endif