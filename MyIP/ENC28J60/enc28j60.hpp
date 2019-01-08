/**
 ******************************************************************************
 * @file    enc28j60.hpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    07/01/2019
 * @brief   This file provides all the ENC28J60 firmware method.
 ******************************************************************************
 * @attention
 *
 *
 * <h2><center>&copy; </center></h2>
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ENC28J60_HPP
#define __ENC28J60_HPP

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* User lib */
#include "net.hpp"
#include "virtual_port.hpp"
#include "stm32f10x_gpio.h"


/**
 * @brief Class ENC28J60
 */
class Enc
{
    public:
    
        enum Size_t
        {
            IP_ADDR_SIZE    = 4,
            MAC_ADDR_SIZE   = 6
        };
    
        Enc(VirtualPort*, GPIO_TypeDef*, uint16_t, GPIO_TypeDef*, uint16_t);
    
        void Init(uint8_t*, uint8_t*, uint16_t, size_t sizeBuf = MAX_FRAMELEN);
        
        void InitPhy();
        
        void Task();
        
        bool CreateClass() const;

    private:
    
        /// ENC28J60 Control Registers
        enum EncControlRegisters_t : uint8_t
        {
            // Control register definitions are a combination of address,
            // bank number, and Ethernet/MAC/PHY indicator bits.
            // - Register address        (bits 0-4)
            // - Bank number        (bits 5-6)
            // - MAC/PHY indicator        (bit 7)
            ADDR_MASK   = 0x1F,
            BANK_MASK   = 0x60,
            SPRD_MASK   = 0x80
        };
        
        /// All-bank registers
        enum EncAllBankRegisters_t : uint8_t
        {
            EIE         = 0x1B,
            EIR         = 0x1C,
            ESTAT       = 0x1D,
            ECON2       = 0x1E,
            ECON1       = 0x1F
        };

        /// Bank 0 registers
        enum EncBank0Registers_t
        {
            ERDPTL      = (0x00|0x00),
            ERDPTH      = (0x01|0x00),
            EWRPTL      = (0x02|0x00),
            EWRPTH      = (0x03|0x00),
            ETXSTL      = (0x04|0x00),
            ETXSTH      = (0x05|0x00),
            ETXNDL      = (0x06|0x00),
            ETXNDH      = (0x07|0x00),
            ERXSTL      = (0x08|0x00),
            ERXSTH      = (0x09|0x00),
            ERXNDL      = (0x0A|0x00),
            ERXNDH      = (0x0B|0x00),
            ERXRDPTL    = (0x0C|0x00),
            ERXRDPTH    = (0x0D|0x00),
            ERXWRPTL    = (0x0E|0x00),
            ERXWRPTH    = (0x0F|0x00),
            EDMASTL     = (0x10|0x00),
            EDMASTH     = (0x11|0x00),
            EDMANDL     = (0x12|0x00),
            EDMANDH     = (0x13|0x00),
            EDMADSTL    = (0x14|0x00),
            EDMADSTH    = (0x15|0x00),
            EDMACSL     = (0x16|0x00),
            EDMACSH     = (0x17|0x00)
        };
        
        /// Bank 1 registers
        enum EncBank1Registers_t
        {
            EHT0        = (0x00|0x20),
            EHT1        = (0x01|0x20),
            EHT2        = (0x02|0x20),
            EHT3        = (0x03|0x20),
            EHT4        = (0x04|0x20),
            EHT5        = (0x05|0x20),
            EHT6        = (0x06|0x20),
            EHT7        = (0x07|0x20),
            EPMM0       = (0x08|0x20),
            EPMM1       = (0x09|0x20),
            EPMM2       = (0x0A|0x20),
            EPMM3       = (0x0B|0x20),
            EPMM4       = (0x0C|0x20),
            EPMM5       = (0x0D|0x20),
            EPMM6       = (0x0E|0x20),
            EPMM7       = (0x0F|0x20),
            EPMCSL      = (0x10|0x20),
            EPMCSH      = (0x11|0x20),
            EPMOL       = (0x14|0x20),
            EPMOH       = (0x15|0x20),
            EWOLIE      = (0x16|0x20),
            EWOLIR      = (0x17|0x20),
            ERXFCON     = (0x18|0x20),
            EPKTCNT     = (0x19|0x20)
        };
        
        /// Bank 2 registers
        enum EncBank2Registers_t
        {
            MACON1      = (0x00|0x40|0x80),
            MACON2      = (0x01|0x40|0x80),
            MACON3      = (0x02|0x40|0x80),
            MACON4      = (0x03|0x40|0x80),
            MABBIPG     = (0x04|0x40|0x80),
            MAIPGL      = (0x06|0x40|0x80),
            MAIPGH      = (0x07|0x40|0x80),
            MACLCON1    = (0x08|0x40|0x80),
            MACLCON2    = (0x09|0x40|0x80),
            MAMXFLL     = (0x0A|0x40|0x80),
            MAMXFLH     = (0x0B|0x40|0x80),
            MAPHSUP     = (0x0D|0x40|0x80),
            MICON       = (0x11|0x40|0x80),
            MICMD       = (0x12|0x40|0x80),
            MIREGADR    = (0x14|0x40|0x80),
            MIWRL       = (0x16|0x40|0x80),
            MIWRH       = (0x17|0x40|0x80),
            MIRDL       = (0x18|0x40|0x80),
            MIRDH       = (0x19|0x40|0x80)
        };
        
        /// Bank 3 registers
        enum EncBank3Registers_t
        {
            MAADR1      = (0x00|0x60|0x80),
            MAADR0      = (0x01|0x60|0x80),
            MAADR3      = (0x02|0x60|0x80),
            MAADR2      = (0x03|0x60|0x80),
            MAADR5      = (0x04|0x60|0x80),
            MAADR4      = (0x05|0x60|0x80),
            EBSTSD      = (0x06|0x60),
            EBSTCON     = (0x07|0x60),
            EBSTCSL     = (0x08|0x60),
            EBSTCSH     = (0x09|0x60),
            MISTAT      = (0x0A|0x60|0x80),
            EREVID      = (0x12|0x60),
            ECOCON      = (0x15|0x60),
            EFLOCON     = (0x17|0x60),
            EPAUSL      = (0x18|0x60),
            EPAUSH      = (0x19|0x60)
        };
        
        // PHY registers
        enum EncPhyRegisters_t : uint8_t
        {
            PHCON1      = 0x00,
            PHSTAT1     = 0x01,
            PHHID1      = 0x02,
            PHHID2      = 0x03,
            PHCON2      = 0x10,
            PHSTAT2     = 0x11,
            PHIE        = 0x12,
            PHIR        = 0x13,
            PHLCON      = 0x14
        };
        
        /// ENC28J60 ERXFCON Register Bit Definitions
        enum EncErxfconRegistersBitDefinitions_t : uint8_t
        {
            ERXFCON_UCEN    = 0x80,
            ERXFCON_ANDOR   = 0x40,
            ERXFCON_CRCEN   = 0x20,
            ERXFCON_PMEN    = 0x10,
            ERXFCON_MPEN    = 0x08,
            ERXFCON_HTEN    = 0x04,
            ERXFCON_MCEN    = 0x02,
            ERXFCON_BCEN    = 0x01
        };
        
        /// ENC28J60 EIE Register Bit Definitions
        enum EncEieRegistersBitDefinitions_t : uint8_t
        {
            EIE_INTIE   = 0x80,
            EIE_PKTIE   = 0x40,
            EIE_DMAIE   = 0x20,
            EIE_LINKIE  = 0x10,
            EIE_TXIE    = 0x08,
            EIE_WOLIE   = 0x04,
            EIE_TXERIE  = 0x02,
            EIE_RXERIE  = 0x01
        };
        
        /// ENC28J60 EIR Register Bit Definitions
        enum EncEirRegistersBitDefinitions_t : uint8_t
        {
            EIR_PKTIF   = 0x40,
            EIR_DMAIF   = 0x20,
            EIR_LINKIF  = 0x10,
            EIR_TXIF    = 0x08,
            EIR_WOLIF   = 0x04,
            EIR_TXERIF  = 0x02,
            EIR_RXERIF  = 0x01
        };

        /// ENC28J60 ESTAT Register Bit Definitions
        enum EncEstatRegistersBitDefinitions_t : uint8_t
        {
            ESTAT_INT       = 0x80,
            ESTAT_LATECOL   = 0x10,
            ESTAT_RXBUSY    = 0x04,
            ESTAT_TXABRT    = 0x02,
            ESTAT_CLKRDY    = 0x01
        };
        
        /// ENC28J60 ECONs Register Bit Definitions
        enum EncEconsRegistersBitDefinitions_t : uint8_t
        {
            ECON2_AUTOINC   = 0x80,
            ECON2_PKTDEC    = 0x40,
            ECON2_PWRSV     = 0x20,
            ECON2_VRPS      = 0x08,
                
            ECON1_TXRST     = 0x80,
            ECON1_RXRST     = 0x40,
            ECON1_DMAST     = 0x20,
            ECON1_CSUMEN    = 0x10,
            ECON1_TXRTS     = 0x08,
            ECON1_RXEN      = 0x04,
            ECON1_BSEL1     = 0x02,
            ECON1_BSEL0     = 0x01
        };
        
        /// ENC28J60 MACONs Register Bit Definitions
        enum EncMaconsRegistersBitDefinitions_t : uint8_t
        {
            MACON1_LOOPBK   = 0x10,
            MACON1_TXPAUS   = 0x08,
            MACON1_RXPAUS   = 0x04,
            MACON1_PASSALL  = 0x02,
            MACON1_MARXEN   = 0x01,

            MACON2_MARST    = 0x80,
            MACON2_RNDRST   = 0x40,
            MACON2_MARXRST  = 0x08,
            MACON2_RFUNRST  = 0x04,
            MACON2_MATXRST  = 0x02,
            MACON2_TFUNRST  = 0x01,

            MACON3_PADCFG2  = 0x80,
            MACON3_PADCFG1  = 0x40,
            MACON3_PADCFG0  = 0x20,
            MACON3_TXCRCEN  = 0x10,
            MACON3_PHDRLEN  = 0x08,
            MACON3_HFRMLEN  = 0x04,
            MACON3_FRMLNEN  = 0x02,
            MACON3_FULDPX   = 0x01
        };
        
        /// ENC28J60 MICMD Register Bit Definitions
        enum EncMicmdDRegistersBitDefinitions_t : uint8_t
        {
            MICMD_MIISCAN   = 0x02,
            MICMD_MIIRD     = 0x01
        };

        /// ENC28J60 MISTAT Register Bit Definitions
        enum EncMistatRegistersBitDefinitions_t : uint8_t
        {
            MISTAT_NVALID   = 0x04,
            MISTAT_SCAN     = 0x02,
            MISTAT_BUSY     = 0x01
        };
        
        /// ENC28J60 PHY PHCON1 Register Bit Definitions
        enum EncPhyPhcon1RegistersBitDefinitions_t : uint16_t
        {
            PHCON1_PRST     = 0x8000,
            PHCON1_PLOOPBK  = 0x4000,
            PHCON1_PPWRSV   = 0x0800,
            PHCON1_PDPXMD   = 0x0100
        };
        
        /// ENC28J60 PHY PHSTAT1 Register Bit Definitions
        enum EncPhyPhstat1RegistersBitDefinitions_t : uint16_t
        {
            PHSTAT1_PFDPX   = 0x1000,
            PHSTAT1_PHDPX   = 0x0800,
            PHSTAT1_LLSTAT  = 0x0004,
            PHSTAT1_JBSTAT  = 0x0002
        };
        
        /// ENC28J60 PHY PHCON2 Register Bit Definitions
        enum EncPhyPhcon2RegistersBitDefinitions_t : uint16_t
        {
            PHCON2_FRCLINK  = 0x4000,
            PHCON2_TXDIS    = 0x2000,
            PHCON2_JABBER   = 0x0400,
            PHCON2_HDLDIS   = 0x0100
        };
        
        /// ENC28J60 Packet Control Byte Bit Definitions
        enum EncPacketControlByteBitDefinitions_t : uint8_t
        {
            PKTCTRL_PHUGEEN     = 0x08,
            PKTCTRL_PPADEN      = 0x04,
            PKTCTRL_PCRCEN      = 0x02,
            PKTCTRL_POVERRIDE   = 0x01
        };
        
        /// SPI operation codes
        enum EncOperationCodes_t : uint8_t
        {
            ENC28J60_READ_CTRL_REG  = 0x00,
            ENC28J60_READ_BUF_MEM   = 0x3A,
            ENC28J60_WRITE_CTRL_REG = 0x40,
            ENC28J60_WRITE_BUF_MEM  = 0x7A,
            ENC28J60_BIT_FIELD_SET  = 0x80,
            ENC28J60_BIT_FIELD_CLR  = 0xA0,
            ENC28J60_SOFT_RESET     = 0xFF
        };

        enum Default_t
        {
            // The RXSTART_INIT should be zero. See Rev. B4 Silicon Errata
            // buffer boundaries applied to internal 8K ram
            // the entire available packet buffer space is allocated
            //
            // start with recbuf at 0/
            RXSTART_INIT        = 0x0,
            // receive buffer end
            RXSTOP_INIT         = (0x1FFF-0x0600-1),
            // start TX buffer at 0x1FFF-0x0600, pace for one full ethernet frame (~1500 bytes)
            TXSTART_INIT        = (0x1FFF-0x0600),
            // stp TX buffer at end of mem
            TXSTOP_INIT         = 0x1FFF,
            // max frame length which the conroller will accept:
            MAX_FRAMELEN        = 1500,        ///< (note: maximum ethernet frame length would be 1518)
                
            INITIAL_TCP_SEQUENCE_NUMBER = 0x0A  ///< my initial tcp sequence number
        };
        
        
        struct InterfaceSettings_t
        {
            GPIO_TypeDef* CS_Port;
            GPIO_InitTypeDef CS_Pin;
            GPIO_TypeDef* RESET_Port;
            GPIO_InitTypeDef RESET_Pin;

            InterfaceSettings_t()
            {
                CS_Port = nullptr;
                RESET_Port = nullptr;
            };
        };
        
        enum Enc_t
        {
            ENC_1,
            ENC_2,
            ENC_MAX_COUNT 
        };
        
        void CsEnable() const;
        
        void CsDisable() const;
        
        void HardReset() const;
        
        void PhyWrite(uint8_t, uint16_t);
        
        void WriteReg(uint8_t, uint8_t);
        
        uint8_t ReadReg(uint8_t);
        
        void SetBank(uint8_t);
        
        void WriteOp(uint8_t, uint8_t, uint8_t) const;
        
        uint8_t ReadOp(uint8_t, uint8_t) const;
        
        void ReadBuffer(uint8_t*, size_t);
        
        void WriteBuffer(const uint8_t*, size_t);
        
        void SetInterfaceSettings(InterfaceSettings_t&);
        
        void GetInterfaceSettings(InterfaceSettings_t* const) const;
        
        void InitGpio(InterfaceSettings_t& settings) const;
        
        size_t PacketReceive(uint8_t*, size_t);
        
        void PacketSend(const uint8_t*, size_t);
        
        uint8_t Enc28j60Bank;
        
        size_t NextPacketPtr;
        
        uint8_t TcpPort;
        
        uint8_t MacAddr[MAC_ADDR_SIZE];
        
        uint8_t IpAddr[IP_ADDR_SIZE];
        
        int InfoHdrLen;
        
        int InfoDataLen;
        
        uint8_t Seqnum;
        
        uint16_t IpIdentifier;
        
        uint8_t *Buf;
        
        size_t BufSize;
        
        InterfaceSettings_t* InterfaceSettings; ///< Interface
        
        VirtualPort* VPort;     ///< Port SPI

        static Enc* Encs[ENC_MAX_COUNT];     ///< Static instances of a class
};

#endif