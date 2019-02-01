/**
 ******************************************************************************
 * @file    virtual_port.hpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    10-April-2018
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
#ifndef __VIRTUAL_PORT_HPP
#define __VIRTUAL_PORT_HPP

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus

class VirtualPort {
  public:
    /// Transmits single data
    virtual void Transmit(const uint8_t*, size_t) = 0;

    /// Returns the most recent received data
    virtual size_t Receive(uint8_t*, size_t) = 0;

    virtual void ClearTransmit() = 0;

    virtual void ClearReceive() = 0;

    virtual size_t GetLen() = 0;

    virtual void WaitingCompleteTransfer() = 0;
};

extern "C" {
}
#endif

#endif