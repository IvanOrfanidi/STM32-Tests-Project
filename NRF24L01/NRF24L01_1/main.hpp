/**
 ******************************************************************************
 * @file    rtc.hpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    28-March-2018
 * @brief   This file contains all the methods prototypes for the RTC
 *          firmware library.
 ******************************************************************************
 * @attention
 *
 *
 *
 ******************************************************************************
**/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_HPP
#define __MAIN_HPP

/* Includes ------------------------------------------------------------------*/
#include "system.hpp"
#include "board.hpp"
#include "stm32f10x_conf.h"
#include "uart.hpp"
#include "spi.hpp"
#include "exti.hpp"
#include "virtual_port.hpp"
#include "nrf24l01.hpp"
#include "rtc.hpp"
#include "sdcard.hpp"

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus

class Main {
  public:
    enum Timeout_t {
        TEST_CHANNEL = 500,
    };

    Main();

    static void NrfTask();

    bool ChannelBusy(uint8_t);

    char AddrNrf[Nrf::MAX_SIZE_ADDRESS];

    class Nrf* txSingle;

    class VirtualPort* VPortUart;
};

extern "C" {
}

#endif    //__cplusplus

#endif