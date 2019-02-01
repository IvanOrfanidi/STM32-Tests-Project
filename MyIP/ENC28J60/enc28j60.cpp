/**
 ******************************************************************************
 * @file    enc28j60.cpp
 * @author  Ivan Orfanidi
 * @version V1.0.0
 * @date    07/01/2019
 * @brief   This file provides all the enc28j60 firmware method.
 ******************************************************************************
 * @attention
 *
 *
 * <h2><center>&copy; </center></h2>
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "enc28j60.hpp"
#include "board.hpp"

Enc* Enc::Encs[ENC_MAX_COUNT];

/**
 * @brief Ñonstructor
 * @param [in] port - virtual port (SPI)
 * @param [in] cs_port - chip select port
 * @param [in] cs_pin - chip select pin
 * @param [in] reset_port - chip reset port
 * @param [in] reset_pin - chip reset pin
 */
Enc::Enc(VirtualPort* port, GPIO_TypeDef* cs_port, uint16_t cs_pin, GPIO_TypeDef* reset_port, uint16_t reset_pin)
{
    InterfaceSettings_t newSettings;
    newSettings.CS_Port = cs_port;
    newSettings.RESET_Port = reset_port;
    newSettings.CS_Pin.GPIO_Pin = cs_pin;
    newSettings.RESET_Pin.GPIO_Pin = reset_pin;
    VPort = port;

    Enc28j60Bank = 0x00;
    NextPacketPtr = RXSTART_INIT;
    TcpPort = 0;
    memset(MacAddr, 0, MAC_ADDR_SIZE);
    memset(IpAddr, 0, IP_ADDR_SIZE);
    InfoHdrLen = 0;
    InfoDataLen = 0;
    Seqnum = INITIAL_TCP_SEQUENCE_NUMBER;
    IpIdentifier = 1;
    Buf = nullptr;
    BufSize = 0;

    // Def settings Pin
    newSettings.CS_Pin.GPIO_Mode = GPIO_Mode_Out_PP;
    newSettings.CS_Pin.GPIO_Speed = GPIO_Speed_10MHz;
    newSettings.RESET_Pin.GPIO_Mode = GPIO_Mode_Out_PP;
    newSettings.RESET_Pin.GPIO_Speed = GPIO_Speed_10MHz;

    InterfaceSettings = nullptr;
    size_t freeClass = ENC_MAX_COUNT;
    for(size_t i = 0; i < ENC_MAX_COUNT; i++) {
        if(Encs[i]) {
            InterfaceSettings_t encSettings;
            Encs[i]->GetInterfaceSettings(&encSettings);
            if(((memcmp(&encSettings.CS_Pin, &newSettings.CS_Pin, sizeof(GPIO_InitTypeDef)) == 0) &&
                   (encSettings.CS_Port == newSettings.CS_Port)) ||
                ((memcmp(&encSettings.RESET_Pin, &newSettings.RESET_Pin, sizeof(GPIO_InitTypeDef)) == 0) &&
                    (encSettings.RESET_Port == newSettings.RESET_Port))) {
                /// Error, class was create
                return;
            }
        }
        else {
            freeClass = i;
        }
    }

    if(ENC_MAX_COUNT == freeClass) {
        return;
    }

    SetInterfaceSettings(newSettings);

    // Config GPIO(CS, RESET)
    InitGpio(newSettings);

    Encs[freeClass] = this;
}

void Enc::Init(uint8_t* macAddr, uint8_t* ipAddr, uint16_t tcpPort, size_t sizeBuf)
{
    HardReset();
    WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
    Board::DelayMS(100);

    // Rx start
    WriteReg(ERXSTL, RXSTART_INIT & 0xFF);
    WriteReg(ERXSTH, RXSTART_INIT >> 8);
    // set receive pointer address
    WriteReg(ERXRDPTL, RXSTART_INIT & 0xFF);
    WriteReg(ERXRDPTH, RXSTART_INIT >> 8);
    // RX end
    WriteReg(ERXNDL, RXSTOP_INIT & 0xFF);
    WriteReg(ERXNDH, RXSTOP_INIT >> 8);
    // TX start
    WriteReg(ETXSTL, TXSTART_INIT & 0xFF);
    WriteReg(ETXSTH, TXSTART_INIT >> 8);
    // TX end
    WriteReg(ETXNDL, TXSTOP_INIT & 0xFF);
    WriteReg(ETXNDH, TXSTOP_INIT >> 8);
    // do bank 1 stuff, packet filter:
    // For broadcast packets we allow only ARP packtets
    // All other packets should be unicast only for our mac (MAADR)
    //
    // The pattern to match on is therefore
    // Type     ETH.DST
    // ARP      BROADCAST
    // 06 08 -- ff ff ff ff ff ff -> ip checksum for theses bytes=f7f9
    // in binary these poitions are:11 0000 0011 1111
    // This is hex 303F->EPMM0=0x3f,EPMM1=0x30
    WriteReg(ERXFCON, ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_PMEN);
    WriteReg(EPMM0, 0x3f);
    WriteReg(EPMM1, 0x30);
    WriteReg(EPMCSL, 0xf9);
    WriteReg(EPMCSH, 0xf7);
    //
    //
    // do bank 2 stuff
    // enable MAC receive
    WriteReg(MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
    // bring MAC out of reset
    WriteReg(MACON2, 0x00);
    // enable automatic padding to 60bytes and CRC operations
    WriteOp(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN);
    // set inter-frame gap (non-back-to-back)
    WriteReg(MAIPGL, 0x12);
    WriteReg(MAIPGH, 0x0C);
    // set inter-frame gap (back-to-back)
    WriteReg(MABBIPG, 0x12);
    // Set the maximum packet size which the controller will accept
    // Do not send packets longer than MAX_FRAMELEN:
    WriteReg(MAMXFLL, MAX_FRAMELEN & 0xFF);
    WriteReg(MAMXFLH, MAX_FRAMELEN >> 8);
    // do bank 3 stuff
    // write MAC address
    // NOTE: MAC address in ENC28J60 is byte-backward
    WriteReg(MAADR5, macAddr[0]);
    WriteReg(MAADR4, macAddr[1]);
    WriteReg(MAADR3, macAddr[2]);
    WriteReg(MAADR2, macAddr[3]);
    WriteReg(MAADR1, macAddr[4]);
    WriteReg(MAADR0, macAddr[5]);
    // no loopback of transmitted frames
    PhyWrite(PHCON2, PHCON2_HDLDIS);
    // switch to bank 0
    SetBank(ECON1);
    // enable interrutps
    WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE | EIE_PKTIE);
    // enable packet reception
    WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);

    InitPhy();

    memcpy(MacAddr, macAddr, MAC_ADDR_SIZE);
    memcpy(IpAddr, ipAddr, IP_ADDR_SIZE);
    TcpPort = tcpPort;

    if(sizeBuf > MAX_FRAMELEN) {
        sizeBuf = MAX_FRAMELEN;
    }

    /* Create memory */
    Buf = new uint8_t[sizeBuf];
    BufSize = sizeBuf;
}

void Enc::InitPhy()
{
    /* Magjack leds configuration, see enc28j60 datasheet, page 11 */
    // LEDA=green LEDB=yellow
    //
    // 0x880 is PHLCON LEDB=on, LEDA=on
    // enc28j60PhyWrite(PHLCON,0b0000 1000 1000 00 00);
    PhyWrite(PHLCON, 0x880);
    Board::DelayMS(500);
    //
    // 0x990 is PHLCON LEDB=off, LEDA=off
    // enc28j60PhyWrite(PHLCON,0b0000 1001 1001 00 00);
    PhyWrite(PHLCON, 0x990);
    Board::DelayMS(500);
    //
    // 0x880 is PHLCON LEDB=on, LEDA=on
    // enc28j60PhyWrite(PHLCON,0b0000 1000 1000 00 00);
    PhyWrite(PHLCON, 0x880);
    Board::DelayMS(500);
    //
    // 0x990 is PHLCON LEDB=off, LEDA=off
    // enc28j60PhyWrite(PHLCON,0b0000 1001 1001 00 00);
    PhyWrite(PHLCON, 0x990);
    Board::DelayMS(500);
    //
    // 0x476 is PHLCON LEDA=links status, LEDB=receive/transmit
    // enc28j60PhyWrite(PHLCON,0b0000 0100 0111 01 10);
    PhyWrite(PHLCON, 0x476);
    Board::DelayMS(100);
}

void Enc::PhyWrite(uint8_t address, uint16_t data)
{
    // set the PHY register address
    WriteReg(MIREGADR, address);
    // write the PHY data
    WriteReg(MIWRL, data);
    WriteReg(MIWRH, data >> 8);
    // wait until the PHY write completes
    while(ReadReg(MISTAT) & MISTAT_BUSY) {
        Board::DelayMS(1);
    }
}

void Enc::WriteReg(uint8_t address, uint8_t data)
{
    // set the bank
    SetBank(address);
    // do the write
    WriteOp(ENC28J60_WRITE_CTRL_REG, address, data);
}

uint8_t Enc::ReadReg(uint8_t address)
{
    // set the bank
    SetBank(address);
    // do the read
    return ReadOp(ENC28J60_READ_CTRL_REG, address);
}

void Enc::SetBank(uint8_t address)
{
    // set the bank (if needed)
    if((address & BANK_MASK) != Enc28j60Bank) {
        // set the bank
        WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1 | ECON1_BSEL0));
        WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK) >> 5);
        Enc28j60Bank = (address & BANK_MASK);
    }
}

uint8_t Enc::ReadOp(uint8_t op, uint8_t address) const
{
    uint8_t byte;
    CsEnable();

    byte = op | (address & ADDR_MASK);
    VPort->Transmit((uint8_t*)&byte, sizeof(uint8_t));
    while(VPort->GetLen() == 0)
        ;
    VPort->Receive((uint8_t*)&byte, sizeof(uint8_t));

    byte = 0x00;
    VPort->Transmit((uint8_t*)&byte, sizeof(uint8_t));
    while(VPort->GetLen() == 0)
        ;
    VPort->Receive((uint8_t*)&byte, sizeof(uint8_t));

    // do dummy read if needed (for mac and mii, see datasheet page 29)
    if(address & 0x80) {
        byte = 0x00;
        VPort->Transmit((uint8_t*)&byte, sizeof(uint8_t));
        while(VPort->GetLen() == 0)
            ;
        VPort->Receive((uint8_t*)&byte, sizeof(uint8_t));
    }

    CsDisable();
    return byte;
}

void Enc::WriteOp(uint8_t op, uint8_t address, uint8_t data) const
{
    CsEnable();

    uint8_t byte;
    byte = op | (address & ADDR_MASK);
    VPort->Transmit((uint8_t*)&byte, sizeof(uint8_t));
    while(VPort->GetLen() == 0)
        ;
    VPort->Receive((uint8_t*)&byte, sizeof(uint8_t));

    byte = data;
    VPort->Transmit((uint8_t*)&byte, sizeof(uint8_t));
    while(VPort->GetLen() == 0)
        ;
    VPort->Receive((uint8_t*)&byte, sizeof(uint8_t));

    CsDisable();
}

void Enc::WriteBuffer(const uint8_t* data, size_t len)
{
    CsEnable();
    uint8_t byte;
    byte = ENC28J60_WRITE_BUF_MEM;
    VPort->Transmit((uint8_t*)&byte, sizeof(uint8_t));
    while(VPort->GetLen() == 0)
        ;
    VPort->Receive((uint8_t*)&byte, sizeof(uint8_t));
    while(len--) {
        byte = *data++;
        VPort->Transmit((uint8_t*)&byte, sizeof(uint8_t));
        while(VPort->GetLen() == 0)
            ;
        VPort->Receive((uint8_t*)&byte, sizeof(uint8_t));
    }

    CsDisable();
}

void Enc::ReadBuffer(uint8_t* data, size_t len)
{
    CsEnable();

    uint8_t byte;
    byte = ENC28J60_READ_BUF_MEM;
    VPort->Transmit((uint8_t*)&byte, sizeof(uint8_t));
    while(VPort->GetLen() == 0)
        ;
    VPort->Receive((uint8_t*)&byte, sizeof(uint8_t));

    while(len--) {
        byte = 0x00;
        VPort->Transmit((uint8_t*)&byte, sizeof(uint8_t));
        while(VPort->GetLen() == 0)
            ;
        VPort->Receive((uint8_t*)&byte, sizeof(uint8_t));
        *data++ = byte;
    }
    //*data = '\0';

    CsDisable();
}

// Gets a packet from the network receive buffer, if one is available.
// The packet will be headed by an ethernet header.
//      maxlen  The maximum acceptable length of a retrieved packet.
//      packet  Pointer where packet data should be stored.
// Returns: Packet length in bytes if a packet was retrieved, zero otherwise.
size_t Enc::PacketReceive(uint8_t* packet, size_t maxlen)
{
    // check if a packet has been received and buffered
    //if( !(enc28j60Read(EIR) & EIR_PKTIF) ){
    // The above does not work. See Rev. B4 Silicon Errata point 6.
    if(ReadReg(EPKTCNT) == 0) {
        return 0;
    }

    // Set the read pointer to the start of the received packet
    WriteReg(ERDPTL, NextPacketPtr);
    WriteReg(ERDPTH, NextPacketPtr >> 8);

    // read the next packet pointer
    NextPacketPtr = ReadOp(ENC28J60_READ_BUF_MEM, 0);
    NextPacketPtr |= ReadOp(ENC28J60_READ_BUF_MEM, 0) << 8;

    // read the packet length (see datasheet page 43)
    size_t len = ReadOp(ENC28J60_READ_BUF_MEM, 0);
    len |= ReadOp(ENC28J60_READ_BUF_MEM, 0) << 8;
    len -= 4;    //remove the CRC count

    // read the receive status (see datasheet page 43)
    uint16_t rxstat = ReadOp(ENC28J60_READ_BUF_MEM, 0);
    rxstat |= ReadOp(ENC28J60_READ_BUF_MEM, 0) << 8;

    // limit retrieve length
    if(len > maxlen - 1) {
        len = maxlen - 1;
    }
    // check CRC and symbol errors (see datasheet page 44, table 7-3):
    // The ERXFCON.CRCEN is set by default. Normally we should not
    // need to check this.
    if((rxstat & 0x80) == 0) {
        // invalid
        len = 0;
    }
    else {
        // copy the packet from the receive buffer
        ReadBuffer(packet, len);
    }

    // Move the RX read pointer to the start of the next received packet
    // This frees the memory we just read out
    WriteReg(ERXRDPTL, NextPacketPtr);
    WriteReg(ERXRDPTH, NextPacketPtr >> 8);

    // decrement the packet counter indicate we are done with this packet
    WriteOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
    return len;
}

void Enc::PacketSend(const uint8_t* packet, size_t len)
{
    // Set the write pointer to start of transmit buffer area
    WriteReg(EWRPTL, TXSTART_INIT & 0xFF);
    WriteReg(EWRPTH, TXSTART_INIT >> 8);
    // Set the TXND pointer to correspond to the packet size given
    WriteReg(ETXNDL, (TXSTART_INIT + len) & 0xFF);
    WriteReg(ETXNDH, (TXSTART_INIT + len) >> 8);
    // write per-packet control byte (0x00 means use macon3 settings)
    WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
    // copy the packet into the transmit buffer
    WriteBuffer(packet, len);
    // send the contents of the transmit buffer onto the network
    WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
    // Reset the transmit logic problem. See Rev. B4 Silicon Errata point 12.
    if((ReadReg(EIR) & EIR_TXERIF)) {
        WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
    }
}

void Enc::CsEnable() const
{
    GPIO_ResetBits(InterfaceSettings->CS_Port, InterfaceSettings->CS_Pin.GPIO_Pin);
}

void Enc::CsDisable() const
{
    GPIO_SetBits(InterfaceSettings->CS_Port, InterfaceSettings->CS_Pin.GPIO_Pin);
}

void Enc::HardReset() const
{
    GPIO_ResetBits(InterfaceSettings->RESET_Port, InterfaceSettings->RESET_Pin.GPIO_Pin);
    Board::DelayMS(1000);
    GPIO_SetBits(InterfaceSettings->RESET_Port, InterfaceSettings->RESET_Pin.GPIO_Pin);
}

/**
 * @brief Get alls settings
 * @param [out] settings - alls interface settings, GPIO
 */
void Enc::GetInterfaceSettings(InterfaceSettings_t* const settings) const
{
    memcpy(settings, InterfaceSettings, sizeof(InterfaceSettings_t));
}

/**
 * @brief Set alls settings
 * @param [in] settings - alls interface settings, SPI and GPIO
 */
void Enc::SetInterfaceSettings(InterfaceSettings_t& settings)
{
    /* Copy Settings */
    InterfaceSettings = new InterfaceSettings_t;
    memcpy(&InterfaceSettings->CS_Pin, &settings.CS_Pin, sizeof(GPIO_InitTypeDef));
    memcpy(&InterfaceSettings->RESET_Pin, &settings.RESET_Pin, sizeof(GPIO_InitTypeDef));
    InterfaceSettings->CS_Port = settings.CS_Port;
    InterfaceSettings->RESET_Port = settings.RESET_Port;
}

/**
 * @brief Initialisation GPIO
 * @param [in] settings - settings GPIO
 */
void Enc::InitGpio(InterfaceSettings_t& settings) const
{
    // Enable Clock Port Output
    Board::GpioClock(InterfaceSettings->CS_Port, ENABLE);
    Board::GpioClock(InterfaceSettings->RESET_Port, ENABLE);

    /* Config GPIO CE & CNS for nRF24 */
    GPIO_Init(InterfaceSettings->CS_Port, &InterfaceSettings->CS_Pin);
    GPIO_Init(InterfaceSettings->RESET_Port, &InterfaceSettings->RESET_Pin);
}

bool Enc::CreateClass() const
{
    return ((InterfaceSettings != nullptr) && (Buf != nullptr));
}

void Enc::Task()
{
    while(true) {
        size_t pacLen = PacketReceive(Buf, BufSize);
        if(0 == pacLen) {
            return;
        }

        // arp is broadcast if unknown but a host may also verify the mac address by sending it to a unicast address
        if(Net::EthTypeIsArp(Buf, pacLen, IpAddr)) {
            size_t ansLel = Net::MakeArpAnswerFromRequest(Buf, pacLen, MacAddr, IpAddr);
            PacketSend(Buf, ansLel);
            continue;
        }

        // check if the ip packet is for us
        if(!(Net::EthTypeIsIp(Buf, pacLen, IpAddr))) {
            continue;
        }

        // ICMP Echo (ping)
        if(Net::EthTypeIsIcmpEcho(Buf, pacLen)) {
            size_t ansLel = Net::MakeIcmpEchoAnswerFromRequest(Buf, pacLen, MacAddr, IpAddr);
            PacketSend(Buf, ansLel);
            continue;
        }
    }
}