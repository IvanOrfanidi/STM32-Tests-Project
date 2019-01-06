/**
 ******************************************************************************
 * @file    nrf24l01.hpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    04/01/2019
 * @brief
 ******************************************************************************
 * @attention
 *
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NRF24L01_HPP
#define __NRF24L01_HPP


/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "virtual_port.hpp"
#include "stm32f10x_spi.h"
#include "stm32f10x_gpio.h"


/**
 * @brief Static nRF24L01
 */
class Nrf
{
    public:
    
        /// nRF24L01 registers
        enum Nrf24Registers_t : uint8_t
        {
            // nRF24L0 instruction definitions
            nRF24_CMD_R_REGISTER       = 0x00, // Register read
            nRF24_CMD_W_REGISTER       = 0x20, // Register write
            nRF24_CMD_R_RX_PAYLOAD     = 0x61, // Read RX payload
            nRF24_CMD_W_TX_PAYLOAD     = 0xA0, // Write TX payload
            nRF24_CMD_FLUSH_TX         = 0xE1, // Flush TX FIFO
            nRF24_CMD_FLUSH_RX         = 0xE2, // Flush RX FIFO
            nRF24_CMD_REUSE_TX_PL      = 0xE3, // Reuse TX payload
            nRF24_CMD_LOCK_UNLOCK      = 0x50, // Lock/unlock exclusive features
            nRF24_CMD_NOP              = 0xFF, // No operation (used for reading status register)
            
            // nRF24L0 register definitions
            nRF24_REG_CONFIG           = 0x00, // Configuration register
            nRF24_REG_EN_AA            = 0x01, // Enable "Auto acknowledgment"
            nRF24_REG_EN_RXADDR        = 0x02, // Enable RX addresses
            nRF24_REG_SETUP_AW         = 0x03, // Setup of address widths
            nRF24_REG_SETUP_RETR       = 0x04, // Setup of automatic retransmit
            nRF24_REG_RF_CH            = 0x05, // RF channel
            nRF24_REG_RF_SETUP         = 0x06, // RF setup register
            nRF24_REG_STATUS           = 0x07, // Status register
            nRF24_REG_OBSERVE_TX       = 0x08, // Transmit observe register
            nRF24_REG_RPD              = 0x09, // Received power detector
            nRF24_REG_RX_ADDR_P0       = 0x0A, // Receive address data pipe 0
            nRF24_REG_RX_ADDR_P1       = 0x0B, // Receive address data pipe 1
            nRF24_REG_RX_ADDR_P2       = 0x0C, // Receive address data pipe 2
            nRF24_REG_RX_ADDR_P3       = 0x0D, // Receive address data pipe 3
            nRF24_REG_RX_ADDR_P4       = 0x0E, // Receive address data pipe 4
            nRF24_REG_RX_ADDR_P5       = 0x0F, // Receive address data pipe 5
            nRF24_REG_TX_ADDR          = 0x10, // Transmit address
            nRF24_REG_RX_PW_P0         = 0x11, // Number of bytes in RX payload in data pipe 0
            nRF24_REG_RX_PW_P1         = 0x12, // Number of bytes in RX payload in data pipe 1
            nRF24_REG_RX_PW_P2         = 0x13, // Number of bytes in RX payload in data pipe 2
            nRF24_REG_RX_PW_P3         = 0x14, // Number of bytes in RX payload in data pipe 3
            nRF24_REG_RX_PW_P4         = 0x15, // Number of bytes in RX payload in data pipe 4
            nRF24_REG_RX_PW_P5         = 0x16, // Number of bytes in RX payload in data pipe 5
            nRF24_REG_FIFO_STATUS      = 0x17, // FIFO status register
            nRF24_REG_DYNPD            = 0x1C, // Enable dynamic payload length
            nRF24_REG_FEATURE          = 0x1D, // Feature register
        };
        
        /// nRF24L01 bits definitions
        enum Nrf24Bits_t : uint8_t
        {
            nRF24_CONFIG_PRIM_RX       = 0x01, // PRIM_RX bit in CONFIG register
            nRF24_CONFIG_PWR_UP        = 0x02, // PWR_UP bit in CONFIG register
            nRF24_FLAG_RX_DR           = 0x40, // RX_DR bit (data ready RX FIFO interrupt)
            nRF24_FLAG_TX_DS           = 0x20, // TX_DS bit (data sent TX FIFO interrupt)
            nRF24_FLAG_MAX_RT          = 0x10, // MAX_RT bit (maximum number of TX retransmits interrupt) 
        };
        
        /// Register masks definitions
        enum Nrf24Masks_t : uint8_t
        {
            nRF24_MASK_REG_MAP         = 0x1F, // Mask bits[4:0] for CMD_RREG and CMD_WREG commands
            nRF24_MASK_CRC             = 0x0C, // Mask for CRC bits [3:2] in CONFIG register
            nRF24_MASK_STATUS_IRQ      = 0x70, // Mask for all IRQ bits in STATUS register
            nRF24_MASK_RF_PWR          = 0x06, // Mask RF_PWR[2:1] bits in RF_SETUP register
            nRF24_MASK_RX_P_NO         = 0x0E, // Mask RX_P_NO[3:1] bits in STATUS register
            nRF24_MASK_DATARATE        = 0x28, // Mask RD_DR_[5,3] bits in RF_SETUP register
            nRF24_MASK_EN_RX           = 0x3F, // Mask ERX_P[5:0] bits in EN_RXADDR register
            nRF24_MASK_RX_PW           = 0x3F, // Mask [5:0] bits in RX_PW_Px register
            nRF24_MASK_RETR_ARD        = 0xF0, // Mask for ARD[7:4] bits in SETUP_RETR register
            nRF24_MASK_RETR_ARC        = 0x0F, // Mask for ARC[3:0] bits in SETUP_RETR register
            nRF24_MASK_RXFIFO          = 0x03, // Mask for RX FIFO status bits [1:0] in FIFO_STATUS register
            nRF24_MASK_TXFIFO          = 0x30, // Mask for TX FIFO status bits [5:4] in FIFO_STATUS register
            nRF24_MASK_PLOS_CNT        = 0xF0, // Mask for PLOS_CNT[7:4] bits in OBSERVE_TX register
            nRF24_MASK_ARC_CNT         = 0x0F, // Mask for ARC_CNT[3:0] bits in OBSERVE_TX registe
        };
        
        // Retransmit delay
        enum RetransmitDelay_t : uint8_t
        {
            nRF24_ARD_NONE   = 0x00, // Dummy value for case when retransmission is not used
            nRF24_ARD_250us  = 0x00,
            nRF24_ARD_500us  = 0x01,
            nRF24_ARD_750us  = 0x02,
            nRF24_ARD_1000us = 0x03,
            nRF24_ARD_1250us = 0x04,
            nRF24_ARD_1500us = 0x05,
            nRF24_ARD_1750us = 0x06,
            nRF24_ARD_2000us = 0x07,
            nRF24_ARD_2250us = 0x08,
            nRF24_ARD_2500us = 0x09,
            nRF24_ARD_2750us = 0x0A,
            nRF24_ARD_3000us = 0x0B,
            nRF24_ARD_3250us = 0x0C,
            nRF24_ARD_3500us = 0x0D,
            nRF24_ARD_3750us = 0x0E,
            nRF24_ARD_4000us = 0x0F
        };
        
        /// Data rate
        enum DataRate_t : uint8_t
        {
            nRF24_DR_250kbps = 0x20, // 250kbps data rate
            nRF24_DR_1Mbps   = 0x00, // 1Mbps data rate
            nRF24_DR_2Mbps   = 0x08  // 2Mbps data rate
        };
        
        /// RF output power in TX mode
        enum RfOutputPower_t : uint8_t
        {
            nRF24_TXPWR_18dBm = 0x00, // -18dBm
            nRF24_TXPWR_12dBm = 0x02, // -12dBm
            nRF24_TXPWR_6dBm  = 0x04, //  -6dBm
            nRF24_TXPWR_0dBm  = 0x06  //   0dBm
        };
        
        /// CRC encoding scheme
        enum CrcEncodingScheme_t : uint8_t
        {
            nRF24_CRC_off   = 0x00, // CRC disabled
            nRF24_CRC_1byte = 0x08, // 1-byte CRC
            nRF24_CRC_2byte = 0x0c  // 2-byte CRC
        };
        
        /// nRF24L01 power control
        enum PowerControl_t : uint8_t
        {
            nRF24_PWR_UP   = 0x02, // Power up
            nRF24_PWR_DOWN = 0x00  // Power down
        };
        
        /// Transceiver mode
        enum TransceiverMode_t : uint8_t
        {
            nRF24_MODE_RX = 0x01, // PRX
            nRF24_MODE_TX = 0x00  // PTX
        };
        
        /// Enumeration of RX pipe addresses and TX address
        enum EnumerationRxPipe_t : uint8_t
        {
            nRF24_PIPE0  = 0x00, // pipe0
            nRF24_PIPE1  = 0x01, // pipe1
            nRF24_PIPE2  = 0x02, // pipe2
            nRF24_PIPE3  = 0x03, // pipe3
            nRF24_PIPE4  = 0x04, // pipe4
            nRF24_PIPE5  = 0x05, // pipe5
            nRF24_PIPETX = 0x06  // TX address (not a pipe in fact)
        };
        
        /// State of auto acknowledgment for specified pipe
        enum StateAutoAcknowledgment_t : uint8_t
        {
            nRF24_AA_OFF = 0x00,
            nRF24_AA_ON  = 0x01
        };
        
        /// Status of the RX FIFO
        enum StatusRxFifo_t : uint8_t
        {
            nRF24_STATUS_RXFIFO_DATA  = 0x00, // The RX FIFO contains data and available locations
            nRF24_STATUS_RXFIFO_EMPTY = 0x01, // The RX FIFO is empty
            nRF24_STATUS_RXFIFO_FULL  = 0x02, // The RX FIFO is full
            nRF24_STATUS_RXFIFO_ERROR = 0x03  // Impossible state: RX FIFO cannot be empty and full at the same time
        };
        
        /// Status of the TX FIFO
        enum StatusTxFifo_t : uint8_t
        {
            nRF24_STATUS_TXFIFO_DATA  = 0x00, // The TX FIFO contains data and available locations
            nRF24_STATUS_TXFIFO_EMPTY = 0x01, // The TX FIFO is empty
            nRF24_STATUS_TXFIFO_FULL  = 0x02, // The TX FIFO is full
            nRF24_STATUS_TXFIFO_ERROR = 0x03  // Impossible state: TX FIFO cannot be empty and full at the same time
        };
        
        /// Result of RX FIFO reading
        enum RXResult_t : uint8_t
        {
            nRF24_RX_PIPE0  = 0x00, // Packet received from the PIPE#0
            nRF24_RX_PIPE1  = 0x01, // Packet received from the PIPE#1
            nRF24_RX_PIPE2  = 0x02, // Packet received from the PIPE#2
            nRF24_RX_PIPE3  = 0x03, // Packet received from the PIPE#3
            nRF24_RX_PIPE4  = 0x04, // Packet received from the PIPE#4
            nRF24_RX_PIPE5  = 0x05, // Packet received from the PIPE#5
            nRF24_RX_EMPTY  = 0xFF  // The RX FIFO is empty
        };
    
        /// Ñonstructor
        Nrf(VirtualPort*, GPIO_TypeDef*, uint16_t, GPIO_TypeDef*, uint16_t);
        
        virtual ~Nrf();   /// Destructor
        
        bool CreateClass() const;   /// Returns the status of class creation

        bool Check() const;

    private:
    
        /// Count nRF24L01
        enum NRF24_t
        {
            NRF24_1,
            NRF24_2,
            NRF24_3,
            
            NRF24_MAX_COUNT
        };
        
        struct InterfaceSettings_t
        {
            GPIO_TypeDef* CE_Port;
            GPIO_InitTypeDef CE_Pin;
            GPIO_TypeDef* CNS_Port;
            GPIO_InitTypeDef CNS_Pin;

            InterfaceSettings_t()
            {
                CE_Port = nullptr;
                CNS_Port = nullptr;
            };
        };
        
        void RxOn() const;
        
        void RxOff() const;
        
        void CsnLow() const;
        
        void CsnHigh() const;
        
        uint8_t SpiSendReceiveData(uint8_t) const;

        void WriteMBReg(uint8_t, const uint8_t*, uint8_t) const;
        
        void ReadMBReg(uint8_t, uint8_t*, uint8_t) const;
        
        void InitGpio(InterfaceSettings_t&) const;      /// Initialisation GPIO

        void DeInitGpio(InterfaceSettings_t*) const;      /// Deinitialisation GPIO

        void DeInitSpi(InterfaceSettings_t*) const;      /// Deinitialisation SPI
        
        void GetInterfaceSettings(InterfaceSettings_t* const) const;    /// Get alls interface settings

        void SetInterfaceSettings(InterfaceSettings_t&);                /// Set alls interface settings 

        static Nrf* Nrf24[NRF24_MAX_COUNT];     ///< Static instances of a class

        InterfaceSettings_t* InterfaceSettings; ///< Interface

        VirtualPort* VPort;
};

#endif
