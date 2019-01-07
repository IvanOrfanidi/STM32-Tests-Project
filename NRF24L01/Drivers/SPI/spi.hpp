/**
 ******************************************************************************
 * @file    spi.hpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    06/01/2019
 * @brief   This file provides all the SPI firmware method.
 ******************************************************************************
 * @attention
 *
 *
 * <h2><center>&copy; </center></h2>
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI_HPP
#define __SPI_HPP

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_spi.h"
#include "virtual_port.hpp"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


class Spi : public VirtualPort
{
    public:
    
        enum Default_t
        {
            RX_ADD_SIZE     = 16,
            RX_BUFFER_SIZE  = 16,
        };
            
        /// Ñonstructor
        Spi(SPI_TypeDef* const, SPI_InitTypeDef* initStruct, size_t rx_size = RX_BUFFER_SIZE);
        
        virtual ~Spi();    /// Destructor

        virtual uint32_t GetLen() override;

        virtual void WaitingCompleteTransfer() override;

        /// Transmits single data
        void Transmit(const uint8_t*, size_t) override;

        virtual void ClearTransmit() override;

        /// Returns the most recent received data
        virtual uint32_t Receive(uint8_t*, size_t) override;

        virtual void ClearReceive() override;

        
    private:
    
        bool InitDataBuf(size_t);
    
        void InitDefSpi() const;  /// Initialisation default SPI

        enum Spis_t
        {
            SPI_1,
            SPI_2,
            SPI_3,

            MAX_COUNT_SPI
        };

        static Spi* Spis[MAX_COUNT_SPI]; ///< Main array pointers of classes Uarts

        SPI_TypeDef* SPIx;  ///< Work SPI

        /* Recept val */
        size_t RxBufSize;
        size_t RxCount;
        uint8_t *pRxBuf;  
};

#endif