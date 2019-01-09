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
#include "stm32f10x_gpio.h"


/**
 * @brief Class nRF24L01
 */
class Nrf
{
    public:
    
        /// nRF24L01 registers
        enum Nrf24Registers_t : uint8_t
        {
            // nRF24L0 instruction definitions
            CMD_R_REGISTER       = 0x00, // Register read
            CMD_W_REGISTER       = 0x20, // Register write
            CMD_R_RX_PAYLOAD     = 0x61, // Read RX payload
            CMD_W_TX_PAYLOAD     = 0xA0, // Write TX payload
            CMD_FLUSH_TX         = 0xE1, // Flush TX FIFO
            CMD_FLUSH_RX         = 0xE2, // Flush RX FIFO
            CMD_REUSE_TX_PL      = 0xE3, // Reuse TX payload
            CMD_LOCK_UNLOCK      = 0x50, // Lock/unlock exclusive features
            CMD_NOP              = 0xFF, // No operation (used for reading status register)
            
            // nRF24L0 register definitions
            REG_CONFIG           = 0x00, // Configuration register
            REG_EN_AA            = 0x01, // Enable "Auto acknowledgment"
            REG_EN_RXADDR        = 0x02, // Enable RX addresses
            REG_SETUP_AW         = 0x03, // Setup of address widths
            REG_SETUP_RETR       = 0x04, // Setup of automatic retransmit
            REG_RF_CH            = 0x05, // RF channel
            REG_RF_SETUP         = 0x06, // RF setup register
            REG_STATUS           = 0x07, // Status register
            REG_OBSERVE_TX       = 0x08, // Transmit observe register
            REG_RPD              = 0x09, // Received power detector
            REG_RX_ADDR_P0       = 0x0A, // Receive address data pipe 0
            REG_RX_ADDR_P1       = 0x0B, // Receive address data pipe 1
            REG_RX_ADDR_P2       = 0x0C, // Receive address data pipe 2
            REG_RX_ADDR_P3       = 0x0D, // Receive address data pipe 3
            REG_RX_ADDR_P4       = 0x0E, // Receive address data pipe 4
            REG_RX_ADDR_P5       = 0x0F, // Receive address data pipe 5
            REG_TX_ADDR          = 0x10, // Transmit address
            REG_RX_PW_P0         = 0x11, // Number of bytes in RX payload in data pipe 0
            REG_RX_PW_P1         = 0x12, // Number of bytes in RX payload in data pipe 1
            REG_RX_PW_P2         = 0x13, // Number of bytes in RX payload in data pipe 2
            REG_RX_PW_P3         = 0x14, // Number of bytes in RX payload in data pipe 3
            REG_RX_PW_P4         = 0x15, // Number of bytes in RX payload in data pipe 4
            REG_RX_PW_P5         = 0x16, // Number of bytes in RX payload in data pipe 5
            REG_FIFO_STATUS      = 0x17, // FIFO status register
            REG_DYNPD            = 0x1C, // Enable dynamic payload length
            REG_FEATURE          = 0x1D, // Feature register
        };
        
        /// nRF24L01 bits definitions
        enum Nrf24Bits_t : uint8_t
        {
            CONFIG_PRIM_RX       = 0x01, // PRIM_RX bit in CONFIG register
            CONFIG_PWR_UP        = 0x02, // PWR_UP bit in CONFIG register
            FLAG_RX_DR           = 0x40, // RX_DR bit (data ready RX FIFO interrupt)
            FLAG_TX_DS           = 0x20, // TX_DS bit (data sent TX FIFO interrupt)
            FLAG_MAX_RT          = 0x10, // MAX_RT bit (maximum number of TX retransmits interrupt) 
        };
        
        /// Register masks definitions
        enum Nrf24Masks_t : uint8_t
        {
            MASK_REG_MAP         = 0x1F, // Mask bits[4:0] for CMD_RREG and CMD_WREG commands
            MASK_CRC             = 0x0C, // Mask for CRC bits [3:2] in CONFIG register
            MASK_STATUS_IRQ      = 0x70, // Mask for all IRQ bits in STATUS register
            MASK_RF_PWR          = 0x06, // Mask RF_PWR[2:1] bits in RF_SETUP register
            MASK_RX_P_NO         = 0x0E, // Mask RX_P_NO[3:1] bits in STATUS register
            MASK_DATARATE        = 0x28, // Mask RD_DR_[5,3] bits in RF_SETUP register
            MASK_EN_RX           = 0x3F, // Mask ERX_P[5:0] bits in EN_RXADDR register
            MASK_RX_PW           = 0x3F, // Mask [5:0] bits in RX_PW_Px register
            MASK_RETR_ARD        = 0xF0, // Mask for ARD[7:4] bits in SETUP_RETR register
            MASK_RETR_ARC        = 0x0F, // Mask for ARC[3:0] bits in SETUP_RETR register
            MASK_RXFIFO          = 0x03, // Mask for RX FIFO status bits [1:0] in FIFO_STATUS register
            MASK_TXFIFO          = 0x30, // Mask for TX FIFO status bits [5:4] in FIFO_STATUS register
            MASK_PLOS_CNT        = 0xF0, // Mask for PLOS_CNT[7:4] bits in OBSERVE_TX register
            MASK_ARC_CNT         = 0x0F, // Mask for ARC_CNT[3:0] bits in OBSERVE_TX registe
        };
        
        /// Register setup of address widths
        enum AddrWidth_t
        {
            ILLEGAL = 0x00,
            BYTES_3 = 0x01,
            BYTES_4 = 0x02,
            BYTES_5 = 0x03
        };
        
        // Retransmit delay
        enum RetransmitDelay_t : uint8_t
        {
            ARD_NONE   = 0x00, // Dummy value for case when retransmission is not used
            ARD_250us  = 0x00,
            ARD_500us  = 0x01,
            ARD_750us  = 0x02,
            ARD_1000us = 0x03,
            ARD_1250us = 0x04,
            ARD_1500us = 0x05,
            ARD_1750us = 0x06,
            ARD_2000us = 0x07,
            ARD_2250us = 0x08,
            ARD_2500us = 0x09,
            ARD_2750us = 0x0A,
            ARD_3000us = 0x0B,
            ARD_3250us = 0x0C,
            ARD_3500us = 0x0D,
            ARD_3750us = 0x0E,
            ARD_4000us = 0x0F
        };
        
        /// Data rate
        enum DataRate_t : uint8_t
        {
            DR_250kbps = 0x20, // 250kbps data rate
            DR_1Mbps   = 0x00, // 1Mbps data rate
            DR_2Mbps   = 0x08  // 2Mbps data rate
        };
        
        /// RF output power in TX mode
        enum RfOutputPower_t : uint8_t
        {
            TXPWR_18dBm   = 0x00, // -18dBm
            TXPWR_12dBm   = 0x02, // -12dBm
            TXPWR_6dBm    = 0x04, //  -6dBm
            TXPWR_0dBm    = 0x06,  //   0dBm
                
            RF24_PA_MIN         = TXPWR_18dBm,
            RF24_PA_LOW         = TXPWR_12dBm,
            RF24_PA_HIGH        = TXPWR_6dBm,
            RF24_PA_MAX         = TXPWR_0dBm
        };
        
        /// CRC encoding scheme
        enum CrcEncodingScheme_t : uint8_t
        {
            CRC_off   = 0x00, // CRC disabled
            CRC_1byte = 0x08, // 1-byte CRC
            CRC_2byte = 0x0C  // 2-byte CRC
        };
        
        /// nRF24L01 power control
        enum PowerControl_t : uint8_t
        {
            PWR_UP   = 0x02, // Power up
            PWR_DOWN = 0x00  // Power down
        };
        
        /// Transceiver mode
        enum TransceiverMode_t : uint8_t
        {
            MODE_RX = 0x01, // PRX
            MODE_TX = 0x00  // PTX
        };
        
        /// Enumeration of RX pipe addresses and TX address
        enum EnumerationRxPipe_t : uint8_t
        {
            PIPE0  = 0x00, // pipe0
            PIPE1  = 0x01, // pipe1
            PIPE2  = 0x02, // pipe2
            PIPE3  = 0x03, // pipe3
            PIPE4  = 0x04, // pipe4
            PIPE5  = 0x05, // pipe5
            PIPETX = 0x06  // TX address (not a pipe in fact)
        };
        
        /// State of auto acknowledgment for specified pipe
        enum StateAutoAcknowledgment_t : uint8_t
        {
            AA_OFF = 0x00,
            AA_ON  = 0x01
        };
        
        /// Status of the RX FIFO
        enum StatusRxFifo_t : uint8_t
        {
            STATUS_RXFIFO_DATA  = 0x00, // The RX FIFO contains data and available locations
            STATUS_RXFIFO_EMPTY = 0x01, // The RX FIFO is empty
            STATUS_RXFIFO_FULL  = 0x02, // The RX FIFO is full
            STATUS_RXFIFO_ERROR = 0x03  // Impossible state: RX FIFO cannot be empty and full at the same time
        };
        
        /// Status of the TX FIFO
        enum StatusTxFifo_t : uint8_t
        {
            STATUS_TXFIFO_DATA  = 0x00, // The TX FIFO contains data and available locations
            STATUS_TXFIFO_EMPTY = 0x01, // The TX FIFO is empty
            STATUS_TXFIFO_FULL  = 0x02, // The TX FIFO is full
            STATUS_TXFIFO_ERROR = 0x03  // Impossible state: TX FIFO cannot be empty and full at the same time
        };
        
        /// Result of RX FIFO reading
        enum RXResult_t : uint8_t
        {
            RX_PIPE0  = 0x00, // Packet received from the PIPE#0
            RX_PIPE1  = 0x01, // Packet received from the PIPE#1
            RX_PIPE2  = 0x02, // Packet received from the PIPE#2
            RX_PIPE3  = 0x03, // Packet received from the PIPE#3
            RX_PIPE4  = 0x04, // Packet received from the PIPE#4
            RX_PIPE5  = 0x05, // Packet received from the PIPE#5
            RX_EMPTY  = 0xFF,  // The RX FIFO is empty
                
            RX_PIPE_ALL = 6
        };
        
        // Result of packet transmission
        enum TXResult_t
        {
            TX_SUCCESS,     // Packet has been transmitted successfully
            TX_ERROR,       // Unknown error
            TX_TIMEOUT,     // It was timeout during packet transmit
            TX_MAXRT        // Transmit failed with maximum auto retransmit count
        };
    
        /// Сonstructor
        Nrf(VirtualPort*, GPIO_TypeDef*, uint16_t, GPIO_TypeDef*, uint16_t);
        
        virtual ~Nrf();   /// Destructor

        void Init();    /// Init radio
        
        bool CreateClass() const;   /// Returns the status of class creation

        bool Check() const;     /// Check radio

        /// Function to transmit data packet
        TXResult_t TransmitPacket(const uint8_t*, uint8_t) const;

        void ClearIRQFlags() const;   /// Clear any pending IRQ flags

        void SetPowerMode(PowerControl_t);  /// Control transceiver power mode

        /// Reset packet lost counter (PLOS_CNT bits in OBSERVER_TX register)
        void ResetPLOS() const;

        void SetRFChannel(uint8_t) const;   /// Set frequency channel

        void FlushTX() const;   /// Flush the TX FIFO

        void FlushRX() const;   /// Flush the RX FIFO

        void WritePayload(const uint8_t*, uint8_t) const; /// Write TX payload

        RXResult_t ReadPayload(uint8_t*, uint8_t*);

        void SetOperationalMode(TransceiverMode_t) const; /// Set transceiver operational mode

        void SetCrcScheme(uint8_t) const;   /// Configure transceiver CRC scheme
        
        /// Set automatic retransmission parameters
        void SetAutoRetr(uint8_t, uint8_t) const;

        void SetAddrWidth(uint8_t) const;   /// Set of address widths
        
        /// Set static RX address for a specified pipe
        void SetAddr(uint8_t, const uint8_t*) const;
        
        /// Configure RF output power in TX mode
        void SetTxPower(uint8_t) const;
        
        /// Configure transceiver data rate
        void SetDataRate(uint8_t) const;
        
        /// Configure a specified RX pipe
        void SetRxPipe(uint8_t, uint8_t, uint8_t) const;
        
        /// Disable specified RX pipe
        void ClosePipe(uint8_t) const;
        
        /// Enable the auto retransmit
        void EnableAA(uint8_t) const;
        
        /// Disable the auto retransmit
        void DisableAA(uint8_t) const;
        
        /// Get value of the STATUS register
        uint8_t GetStatus() const;
        
        /// Get pending IRQ flags
        uint8_t GetIRQFlags() const;
        
        /// Get status of the RX FIFO
        uint8_t GetStatus_RXFIFO() const;
        
        /// Get status of the TX FIFO
        uint8_t GetStatus_TXFIFO() const;
        
        /// Get pipe number for the payload available for reading from RX FIFO
        uint8_t GetRXSource() const;
        
        /// Get auto retransmit statistic
        uint8_t GetRetransmitCounters() const;
        
        /// Put the transceiver to the RX mode
        void RxOn() const;
        
        /// Put the transceiver to the TX mode
        void RxOff() const;

    private:
    
        enum Timouts_t
        {
            WAIT_TIMEOUT    = 0x000FFFFF,
        };
    
        /// Count nRF24L01
        enum NRF24_t
        {
            NRF24_1,
            NRF24_2,
            NRF24_3,
            
            NRF24_MAX_COUNT
        };
        
        /// Default
        enum Default_t : uint8_t
        {
            MAX_COUNT_AUTO_RETRANSMITS  = 3,    ///< Максимальном количество попыток при неудачной отправке
            DEF_RF_CHANNEL              = 2,
            DEF_RF_OUTPUT_POWER         = RF24_PA_MAX,
            DEF_RF_DATA_RATE            = DR_2Mbps,

            DEF_CRC_SCHEME              = CRC_1byte,
            DEF_DYNPD                   = 0,
            DEF_FEATURE                 = 0
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
        
        void WriteReg(uint8_t, uint8_t) const;
        
        uint8_t ReadReg(uint8_t) const;
                
        void CsnLow() const;
        
        void CsnHigh() const;
        
        uint8_t SpiSendReceiveData(uint8_t) const;

        void WriteBuffer(uint8_t, const uint8_t*, uint8_t) const;
        
        void ReadBuffer(uint8_t, uint8_t*, uint8_t) const;
        
        void InitGpio(InterfaceSettings_t&) const;      /// Initialisation GPIO

        void DeInitGpio(InterfaceSettings_t*) const;      /// Deinitialisation GPIO

        void DeInitSpi(InterfaceSettings_t*) const;      /// Deinitialisation SPI
        
        void GetInterfaceSettings(InterfaceSettings_t* const) const;    /// Get alls interface settings

        void SetInterfaceSettings(InterfaceSettings_t&);                /// Set alls interface settings 

        static Nrf* Nrf24l01[NRF24_MAX_COUNT];     ///< Static instances of a class

        InterfaceSettings_t* InterfaceSettings; ///< Interface

        VirtualPort* VPort;         ///< Virtual port SPI
};

#endif
